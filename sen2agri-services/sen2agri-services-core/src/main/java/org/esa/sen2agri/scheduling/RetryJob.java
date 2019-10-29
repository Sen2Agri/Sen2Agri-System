/*
 * Copyright (C) 2018 CS ROMANIA
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
package org.esa.sen2agri.scheduling;

import org.apache.commons.lang3.exception.ExceptionUtils;
import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.commons.ProcessingTopic;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.converters.ProductConverter;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.entities.enums.Status;
import org.esa.sen2agri.web.beans.Query;
import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.springframework.web.context.support.SpringBeanAutowiringSupport;
import ro.cs.tao.datasource.DataSource;
import ro.cs.tao.datasource.DataSourceManager;
import ro.cs.tao.datasource.param.CommonParameterNames;
import ro.cs.tao.datasource.param.DataSourceParameter;
import ro.cs.tao.datasource.remote.FetchMode;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.utils.Triple;
import ro.cs.tao.utils.Tuple;

import java.nio.file.Paths;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.util.*;
import java.util.concurrent.ThreadPoolExecutor;

/**
 * @author Cosmin Cara
 */
public class RetryJob extends DownloadJob {
    private static final Map<Tuple<String, String>, Integer> runningJobs = Collections.synchronizedMap(new HashMap<>());
    private static final Object sharedLock = new Object();

    public RetryJob() {
        super();
    }

    @Override
    public String configKey() { return ConfigurationKeys.SCHEDULED_RETRY_ENABLED; }

    @Override
    public String groupName() { return "Retry"; }

    @Override
    public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
        super.execute(jobExecutionContext);
        SpringBeanAutowiringSupport.processInjectionBasedOnCurrentContext(this);
    }

    @Override
    protected void executeImpl(JobDataMap dataMap) {
        final Site site = (Site) dataMap.get("site");
        if (!site.isEnabled()) {
            return;
        }
        DataSourceConfiguration qCfg = (DataSourceConfiguration) dataMap.get("queryConfig");
        DataSourceConfiguration dldCfg = (DataSourceConfiguration) dataMap.get("downloadConfig");
        final Satellite satellite = qCfg.getSatellite();
        final Tuple<String, String> key = new Tuple<>(site.getName(), satellite.friendlyName());
        final Integer previousCount = runningJobs.get(key);
        if (previousCount != null) {
            if (previousCount > 0) {
                logger.warning(String.format("A job for {site:%s,satellite:%s} is already running [%d products remaining]",
                                             site.getName(), satellite.friendlyName(), previousCount));
                return;
            } else {
                runningJobs.remove(key);
            }
        }
        final boolean downloadEnabled = Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_ENABLED);
        final boolean sensorDownloadEnabled = Config.isFeatureEnabled(site.getId(),
                                                                      String.format(ConfigurationKeys.DOWNLOADER_SENSOR_ENABLED,
                                                                                    satellite.friendlyName().toLowerCase()));
        if (!(downloadEnabled && sensorDownloadEnabled)) {
            logger.info(String.format(MESSAGE, site.getShortName(), satellite.name(), "Download disabled"));
            cleanupJob(site, satellite);
            return;
        }
        if (!Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_ENABLED) ||
                !Config.isFeatureEnabled(site.getId(), String.format(ConfigurationKeys.DOWNLOADER_SENSOR_ENABLED,
                                                                     dldCfg.getSatellite().friendlyName().toLowerCase()))) {
            logger.info(String.format(MESSAGE, site.getShortName(), dldCfg.getSatellite().name(), "Download disabled"));
            return;
        }
        try {
            final String downloadPath = Paths.get(dldCfg.getDownloadPath(), site.getShortName()).toString();
            logger.fine(String.format(MESSAGE, site.getShortName(), dldCfg.getSatellite().name(),
                                      "Retry failed products"));
            retryDownloads(site, qCfg, dldCfg, downloadPath);
            logger.fine(String.format(MESSAGE, site.getShortName(), dldCfg.getSatellite().name(),
                                      "Retry last chance products"));
            retryLastChanceProducts(site, qCfg, dldCfg, downloadPath);

        } catch (Exception e) {
            final String message = ExceptionUtils.getStackTrace(e);
            logger.severe(message);
            sendNotification(ProcessingTopic.PROCESSING_ATTENTION.value(),
                             String.format("Retry site \"%s\"", site.getName()),
                             message);

        } finally {
            cleanupJob(site, satellite);
        }
    }

    private void retryDownloads(Site site, DataSourceConfiguration queryCfg, DataSourceConfiguration downloadCfg, String path) {
        final Set<String> tiles = persistenceManager.getSiteTiles(site, downloadCfg.getSatellite());
        final ProductConverter converter = new ProductConverter();
        try {
            final List<DownloadProduct> productsToRetry = this.persistenceManager.getRetriableProducts(site.getId(), downloadCfg);
            final Tuple<String, String> key = new Tuple<>(site.getName(), downloadCfg.getSatellite().friendlyName());
            if (productsToRetry != null && productsToRetry.size() > 0) {
                runningJobs.put(key, productsToRetry.size());
                final DataSource<?> dataSource = DataSourceManager.getInstance().get(queryCfg.getSatellite().name(),
                                                                                  queryCfg.getDataSourceName());
                final Map<String, Map<String, DataSourceParameter>> parameters = dataSource.getSupportedParameters();
                final Map<String, DataSourceParameter> satelliteParameters = parameters.get(queryCfg.getSatellite().name());
                final boolean canFilterByProduct = satelliteParameters.keySet().stream()
                        .filter(pn -> pn.equals(CommonParameterNames.PRODUCT))
                        .findFirst().orElse(null) != null;
                productsToRetry.forEach(p -> {
                    p.setStatusId(Status.DOWNLOADING);
                    p.setNbRetries((short) (p.getNbRetries() + 1));
                    p.setTimestamp(LocalDateTime.now());
                    this.persistenceManager.save(p);
                });
                final List<EOProduct> newProductList = new ArrayList<>();
                for (DownloadProduct dbProduct : productsToRetry) {
                    final EOProduct product = converter.convertToEntityAttribute(dbProduct);
                    if (canFilterByProduct) {
                        try {
                            final Query query = new Query();
                            query.setUser(queryCfg.getUser());
                            query.setPassword(queryCfg.getPassword());
                            final Map<String, Object> params = new HashMap<>();
                            params.put(CommonParameterNames.PRODUCT, product.getName());
                            query.setValues(params);
                            List<EOProduct> list = this.downloadService.query(site.getId(), query, queryCfg);
                            if (list != null && list.size() == 1) {
                                product.setApproximateSize(list.get(0).getApproximateSize());
                                newProductList.add(product);
                            } else {
                                logger.warning(String.format("Product %s cannot be retried (not found on the remote source)",
                                                             dbProduct.getProductName()));
                            }
                        } catch (Exception ex) {
                            logger.warning(String.format("Cannot determine product size for %s", product.getName()));
                        }
                    } else {
                        newProductList.add(product);
                    }
                }
                final ThreadPoolExecutor worker = Config.getWorkerFor(downloadCfg);
                final int resultsSize = newProductList.size();
                runningJobs.put(key, resultsSize);
                for (int i = 0; i < resultsSize; i++) {
                    final List<EOProduct> subList = newProductList.subList(i, i + 1);
                    worker.submit(
                            new DownloadTask(logger, site, downloadCfg.getSatellite(), subList,
                                             () -> {
                                                 Instant startTime = Instant.now();
                                                 downloadService.download(site.getId(), subList,
                                                                          tiles, path, downloadCfg);
                                                 long seconds = Duration.between(startTime, Instant.now()).getSeconds();
                                                 if (downloadCfg.getFetchMode() == FetchMode.SYMLINK && seconds > 10) {
                                                     sendNotification(ProcessingTopic.PROCESSING_ATTENTION.value(),
                                                                      String.format("Retry site \"%s\"", site.getName()),
                                                                      String.format("Symlink creation took %d seconds", seconds));
                                                 }
                                             }, this::downloadCompleted));
                }
                //this.downloadService.download(site.getId(), newProductList, tiles, path, downloadCfg);
            } else {
                logger.info(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                          "No products to retry"));
            }
        } catch (Exception ex) {
            logger.warning(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                         "Download failed: " + ExceptionUtils.getStackTrace(ex)));
        }
    }

    private void retryLastChanceProducts(Site site, DataSourceConfiguration queryCfg, DataSourceConfiguration downloadCfg, String path) {
        final Set<String> tiles = persistenceManager.getSiteTiles(site, downloadCfg.getSatellite());
        final ProductConverter converter = new ProductConverter();
        try {
            List<DownloadProduct> lastChanceProducts = this.persistenceManager.getLastRetriableProducts(site.getId(), downloadCfg);
            if (lastChanceProducts != null && lastChanceProducts.size() > 0) {
                Integer secondaryDatasourceId = downloadCfg.getSecondaryDatasourceId();
                if (secondaryDatasourceId != null) {
                    final DataSourceConfiguration secondaryDS = persistenceManager.getDataSourceConfiguration(secondaryDatasourceId.shortValue());
                    logger.info(String.format("Retry will be attempted using %s", secondaryDS.getDataSourceName()));
                    final DataSource<?> dataSource = DataSourceManager.getInstance().get(queryCfg.getSatellite().name(),
                                                                                      queryCfg.getDataSourceName());
                    final Map<String, Map<String, DataSourceParameter>> parameters = dataSource.getSupportedParameters();
                    final Map<String, DataSourceParameter> satelliteParameters = parameters.get(queryCfg.getSatellite().name());
                    final boolean canFilterByProduct = satelliteParameters.keySet().stream()
                            .filter(pn -> pn.equals(CommonParameterNames.PRODUCT))
                            .findFirst().orElse(null) != null;
                    lastChanceProducts.forEach(p -> {
                        p.setStatusId(Status.DOWNLOADING);
                        p.setNbRetries((short) (p.getNbRetries() + 1));
                        p.setTimestamp(LocalDateTime.now());
                        this.persistenceManager.save(p);
                    });
                    final List<EOProduct> newProductList = new ArrayList<>();
                    for (DownloadProduct dbProduct : lastChanceProducts) {
                        final EOProduct product = converter.convertToEntityAttribute(dbProduct);
                        if (canFilterByProduct) {
                            try {
                                final Query query = new Query();
                                query.setUser(queryCfg.getUser());
                                query.setPassword(queryCfg.getPassword());
                                final Map<String, Object> params = new HashMap<>();
                                params.put(CommonParameterNames.PRODUCT, product.getName());
                                params.put(CommonParameterNames.FOOTPRINT, product.getGeometry());
                                query.setValues(params);
                                final List<EOProduct> list = this.downloadService.query(site.getId(), query, queryCfg);
                                if (list != null && list.size() == 1) {
                                    product.setApproximateSize(list.get(0).getApproximateSize());
                                    newProductList.add(product);
                                } else {
                                    logger.warning(String.format("Product %s cannot be retried (not found on the remote source)",
                                                                 dbProduct.getProductName()));
                                }
                            } catch (Exception ex) {
                                logger.severe(String.format("Cannot query %s. Reason: %s", product.getName(), ex.getMessage()));
                            }
                        } else {
                            newProductList.add(product);
                        }
                    }
                    final ThreadPoolExecutor worker = Config.getWorkerFor(downloadCfg);
                    final int resultsSize = newProductList.size();
                    synchronized (sharedLock) {
                        final Tuple<String, String> key = new Tuple<>(site.getName(), downloadCfg.getSatellite().friendlyName());
                        runningJobs.put(key, runningJobs.getOrDefault(key, 0) + resultsSize);
                    }
                    for (int i = 0; i < resultsSize; i++) {
                        final List<EOProduct> subList = newProductList.subList(i, i + 1);
                        worker.submit(
                                new DownloadTask(logger, site, downloadCfg.getSatellite(), subList,
                                                 () -> {
                                                     Instant startTime = Instant.now();
                                                     downloadService.download(site.getId(), subList,
                                                                              tiles, path, secondaryDS);
                                                     long seconds = Duration.between(startTime, Instant.now()).getSeconds();
                                                     if (downloadCfg.getFetchMode() == FetchMode.SYMLINK && seconds > 10) {
                                                         sendNotification(ProcessingTopic.PROCESSING_ATTENTION.value(),
                                                                          String.format("Retry site \"%s\"", site.getName()),
                                                                          String.format("Symlink creation took %d seconds", seconds));
                                                     }
                                                 }, this::downloadCompleted));
                    }
                    //this.downloadService.download(site.getId(), products, tiles, path, secondaryDS);
                } else {
                    logger.info(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                              "No secondary data source defined"));
                }
            } else {
                logger.info(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                          "No products to retry"));
            }
        } catch (Exception ex) {
            logger.warning(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                         "Download failed: " + ExceptionUtils.getStackTrace(ex)));
        }
    }

    private void downloadCompleted(Triple<String, String, String> key) {
        synchronized (sharedLock) {
            final Tuple<String, String> jobKey = new Tuple<>(key.getKeyOne(), key.getKeyTwo());
            Integer previousCount = runningJobs.get(jobKey);
            if (previousCount == null) {
                previousCount = 0;
            }
            int count = key.getKeyThree().split(",").length;
            int current = previousCount - count;
            logger.fine(String.format("Download retry completed [%s] - remaining products for {site:'%s',satellite:'%s'}: %d",
                                      key.getKeyThree(), key.getKeyOne(), key.getKeyTwo(), current));
            if (current > 0) {
                runningJobs.put(jobKey, current);
            } else {
                runningJobs.remove(jobKey);
            }
        }
    }

    private void cleanupJob(Site site, Satellite sat) {
        runningJobs.remove(new Tuple<>(site.getName(), sat.friendlyName()));
    }
}
