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

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.Status;
import org.esa.sen2agri.entities.converters.ProductConverter;
import org.esa.sen2agri.web.beans.Query;
import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.springframework.web.context.support.SpringBeanAutowiringSupport;
import ro.cs.tao.datasource.DataSource;
import ro.cs.tao.datasource.DataSourceManager;
import ro.cs.tao.datasource.param.DataSourceParameter;
import ro.cs.tao.eodata.EOProduct;

import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.util.*;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
public class RetryJob extends AbstractJob {

    public RetryJob() {
        super();
    }

    @Override
    public String configKey() { return "scheduled.retry.enabled"; }

    @Override
    public String groupName() { return "Retry"; }

    @Override
    public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
        super.execute(jobExecutionContext);
        SpringBeanAutowiringSupport.processInjectionBasedOnCurrentContext(this);
    }

    @Override
    @SuppressWarnings("unchecked")
    protected void executeImpl(JobDataMap dataMap) {
        //List<Site> sites =  (List<Site>) dataMap.get("sites");
        Site site = (Site) dataMap.get("site");
        DataSourceConfiguration qCfg = (DataSourceConfiguration) dataMap.get("queryConfig");
        DataSourceConfiguration dldCfg = (DataSourceConfiguration) dataMap.get("downloadConfig");
        //for (Site site : sites) {
            if (!site.isEnabled()) {
                return;
            }
            if (!Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_STATE_ENABLED) ||
                    !Config.isFeatureEnabled(site.getId(), String.format(ConfigurationKeys.DOWNLOADER_SITE_STATE_ENABLED,
                            dldCfg.getSatellite().shortName().toLowerCase()))) {
                logger.info(String.format(MESSAGE, site.getShortName(), dldCfg.getSatellite().name(),
                                          "Download disabled"));
                return;
            }
            Config.getWorkerFor(dldCfg).submit(() -> {
                final String downloadPath = Paths.get(dldCfg.getDownloadPath(), site.getShortName()).toString();
                logger.fine(String.format(MESSAGE, site.getShortName(), dldCfg.getSatellite().name(),
                                          "Retry failed products"));
                retryDownloads(site, qCfg, dldCfg, downloadPath);
            });
        //}
    }

    private void retryDownloads(Site site, DataSourceConfiguration queryCfg, DataSourceConfiguration downloadCfg, String path) {
        if (!site.isEnabled()) {
            return;
        }
        if (!Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_STATE_ENABLED) ||
                !Config.isFeatureEnabled(site.getId(), String.format(ConfigurationKeys.DOWNLOADER_SITE_STATE_ENABLED,
                        downloadCfg.getSatellite().shortName().toLowerCase()))) {
            logger.info(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(), "Download disabled"));
            return;
        }
        try {
            List<DownloadProduct> productsToRetry = this.persistenceManager.getRetriableProducts(site.getId(), downloadCfg);
            if (productsToRetry != null && productsToRetry.size() > 0) {
                DataSource dataSource = DataSourceManager.getInstance().get(queryCfg.getSatellite().name(),
                                                                            queryCfg.getDataSourceName());
                Map<String, Map<String, DataSourceParameter>> parameters = dataSource.getSupportedParameters();
                boolean canFilterByProduct = parameters.get(queryCfg.getSatellite().name()).get("product") != null;
                productsToRetry.forEach(p -> {
                    p.setStatusId(Status.DOWNLOADING);
                    p.setNbRetries((short) (p.getNbRetries() + 1));
                    p.setTimestamp(LocalDateTime.now());
                    this.persistenceManager.save(p);
                });
                Set<String> tiles = persistenceManager.getSiteTiles(site, downloadCfg.getSatellite());
                ProductConverter converter = new ProductConverter();
                List<EOProduct> products = productsToRetry.stream()
                        .map(dbp -> {
                            EOProduct product = converter.convertToEntityAttribute(dbp);
                            if (canFilterByProduct) {
                                try {
                                    Query query = new Query();
                                    query.setUser(queryCfg.getUser());
                                    query.setPassword(queryCfg.getPassword());
                                    Map<String, Object> params = new HashMap<>();
                                    params.put("product", product.getName());
                                    query.setValues(params);
                                    List<EOProduct> list = this.downloadService.query(site.getId(), query, queryCfg, -1);
                                    if (list != null && list.size() == 1) {
                                        product.setApproximateSize(list.get(0).getApproximateSize());
                                    }
                                } catch (Exception ex) {
                                    logger.warning(String.format("Cannot determine product size for %s", product.getName()));
                                }
                            }
                            return product;
                        }).collect(Collectors.toList());
                this.downloadService.download(site.getId(), products, tiles, path, downloadCfg);
            } else {
                logger.info(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                          "No products to retry"));
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.warning(String.format(MESSAGE, site.getShortName(), downloadCfg.getSatellite().name(),
                                         "Download failed: " + ex.getMessage() + " @ " + String.join(" < ",
                                                                           Arrays.stream(ex.getStackTrace())
                                                                                   .map(StackTraceElement::toString)
                                                                                   .collect(Collectors.toSet()))));
        }
    }
}
