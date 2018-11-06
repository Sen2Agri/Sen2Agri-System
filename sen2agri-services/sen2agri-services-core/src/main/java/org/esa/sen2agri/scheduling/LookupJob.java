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
import org.esa.sen2agri.commons.Constants;
import org.esa.sen2agri.commons.ParameterHelper;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.entities.*;
import org.esa.sen2agri.web.beans.Query;
import org.quartz.JobDataMap;
import ro.cs.tao.eodata.EOData;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.eodata.Polygon2D;
import ro.cs.tao.utils.Triple;
import ro.cs.tao.utils.Tuple;

import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.Future;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
public class LookupJob extends AbstractJob {

    private static final Map<Tuple<String, String>, Integer> runningJobs = Collections.synchronizedMap(new HashMap<>());

    public LookupJob() {
        super();
    }

    @Override
    public String configKey() {
        return "scheduled.lookup.enabled";
    }

    @Override
    public String groupName() { return "Lookup"; }

    @Override
    @SuppressWarnings("unchecked")
    protected void executeImpl(JobDataMap dataMap) {
        List<Site> sites =  (List<Site>) dataMap.get("sites");
        DataSourceConfiguration queryConfig = (DataSourceConfiguration) dataMap.get("queryConfig");
        DataSourceConfiguration downloadConfig = (DataSourceConfiguration) dataMap.get("downloadConfig");
        for (Site site : sites) {
            if (!site.isEnabled()) {
                continue;
            }
            Satellite satellite = queryConfig.getSatellite();
            Tuple<String, String> key = new Tuple<>(site.getName(), satellite.shortName());
            if (runningJobs.containsKey(key)) {
                logger.warning(String.format("A job is already running for the site '%s'", site.getName()));
                continue;
            }
            boolean downloadEnabled = Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_STATE_ENABLED);
            boolean sensorDownloadEnabled = Config.isFeatureEnabled(site.getId(),
                    String.format(ConfigurationKeys.DOWNLOADER_SITE_STATE_ENABLED,
                            satellite.shortName().toLowerCase()));
            if (!downloadEnabled || !sensorDownloadEnabled) {
                logger.info(String.format(MESSAGE, site.getShortName(), satellite.name(), "Download disabled"));
                cleanupJob(site, satellite);
                continue;
            }
            final short siteId = site.getId();
            final List<Season> seasons = persistenceManager.getEnabledSeasons(siteId);
            if (seasons != null && seasons.size() > 0) {
                seasons.sort(Comparator.comparing(Season::getStartDate));
                logger.fine(String.format(MESSAGE, site.getShortName(), satellite.name(),
                                          "Seasons defined: " +
                                            String.join(";",
                                                        seasons.stream()
                                                                .map(Season::toString)
                                                                .collect(Collectors.toList()))));
                LocalDateTime startDate = getStartDate(satellite, site, seasons);
                LocalDateTime endDate = getEndDate(seasons);
                logger.fine(String.format(MESSAGE, site.getShortName(), satellite.name(),
                        String.format("Using start date: %s and end date: %s", startDate, endDate)));
                downloadEnabled = Config.isFeatureEnabled(site.getId(), ConfigurationKeys.DOWNLOADER_STATE_ENABLED);
                sensorDownloadEnabled = Config.isFeatureEnabled(site.getId(),
                        String.format(ConfigurationKeys.DOWNLOADER_SITE_STATE_ENABLED,
                                satellite.shortName().toLowerCase()));
                if (!downloadEnabled || !sensorDownloadEnabled) {
                    logger.info(String.format(MESSAGE, site.getShortName(), satellite.name(), "Download disabled"));
                    continue;
                }
                final String downloadPath = Paths.get(queryConfig.getDownloadPath(), site.getShortName()).toString();
                if (LocalDateTime.now().compareTo(startDate) >= 0) {
                    if (endDate.compareTo(startDate) >= 0) {
                        logger.fine(String.format(MESSAGE, site.getShortName(), satellite.name(),
                                                  String.format("Lookup for new products in range %s - %s",
                                                                startDate.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                                                                endDate.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)))));
                        lookupAndDownload(site, startDate, endDate, downloadPath, queryConfig, downloadConfig);
                    } else {
                        logger.info(String.format(MESSAGE, site.getName(), satellite.name(),
                                                  "No products to download"));
                        cleanupJob(site, satellite);
                    }
                } else {
                    logger.info(String.format(MESSAGE, site.getName(), satellite.name(), "Season not started"));
                    cleanupJob(site, satellite);
                }
            } else {
                logger.warning(String.format(MESSAGE, site.getName(), satellite.name(),
                                             "No season defined or season not started"));
                cleanupJob(site, satellite);
            }
        }
    }

    protected LocalDateTime getStartDate(Satellite satellite, Site site, List<Season> seasons) {
        LocalDateTime minStartDate = seasons.get(0).getStartDate().atStartOfDay();
        int downloaderStartOffset = Config.getAsInteger(ConfigurationKeys.DOWNLOADER_START_OFFSET, 0);
        if (downloaderStartOffset > 0) {
            minStartDate = minStartDate.minusMonths(downloaderStartOffset);
        }
        LocalDateTime startDate;
        LocalDateTime lastDate = null;
        // Only use the last download date if the flag 'downloader.%s.forcestart' is set to 'false' or not present
        if (!Config.getAsBoolean(site.getId(),
                                 String.format(ConfigurationKeys.DOWNLOADER_SITE_FORCE_START_ENABLED, satellite.shortName()),
                                 false)) {
            lastDate = persistenceManager.getLastDownloadDate(site.getId(), satellite.value());
        }
        startDate = lastDate == null ? minStartDate : lastDate.plusDays(1).toLocalDate().atStartOfDay();
        return startDate;
    }

    protected LocalDateTime getEndDate(List<Season> seasons) {
        LocalDateTime endDate = seasons.get(seasons.size() - 1).getEndDate().atStartOfDay().plusDays(1).minusSeconds(1);
        if (LocalDateTime.now().compareTo(endDate) < 0) {
            endDate = LocalDateTime.now();
        }
        return endDate;
    }

    private void lookupAndDownload(Site site, LocalDateTime start, LocalDateTime end, String path,
                                   DataSourceConfiguration queryConfiguration, DataSourceConfiguration downloadConfiguration) {
        Query query = new Query();
        query.setUser(queryConfiguration.getUser());
        query.setPassword(queryConfiguration.getPassword());
        Map<String, Object> params = new HashMap<>();
        Satellite satellite = queryConfiguration.getSatellite();
        Set<String> tiles = persistenceManager.getSiteTiles(site, downloadConfiguration.getSatellite());
        params.put(ParameterHelper.getStartDateParamName(satellite),
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        params.put(ParameterHelper.getEndDateParamName(satellite),
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        params.put(ParameterHelper.getFootprintParamName(satellite), Polygon2D.fromWKT(site.getExtent()));
        String tileParamName = ParameterHelper.getTileParamName(satellite);
        if (tiles != null && tiles.size() > 0 && tileParamName != null) {
            String tileList;
            if (tiles.size() == 1) {
                tileList = tiles.stream().findFirst().get();
            } else {
                tileList = "[" + String.join(",", tiles) + "]";
            }
            params.put(tileParamName, tileList);
        }
        final List<Parameter> specificParameters = queryConfiguration.getSpecificParameters();
        if (specificParameters != null) {
            specificParameters.forEach(p -> {
                try {
                    params.put(p.getName(), p.typedValue());
                } catch (Exception e) {
                    logger.warning(String.format(MESSAGE, site.getName(), satellite.name(),
                                                 String.format("Incorrect typed value for parameter [%s] : expected '%s', found '%s'",
                                                               p.getName(), p.getType(), p.getValue())));
                }
            });
        }
        query.setValues(params);
        try {
            logger.fine(String.format(MESSAGE, site.getName(), satellite.name(),
                    String.format("Performing query for interval %s - %s",
                            start.toString(), end.toString())));
            Tuple<String, String> key = new Tuple<>(site.getName(), satellite.shortName());
            runningJobs.put(key, 0);
            final Future<Long> futureCount = Config.getWorkerFor(queryConfiguration)
                    .submit(() -> downloadService.count(site.getId(), query, queryConfiguration));
            long count = futureCount.get();
            ProductCount productCount = new ProductCount();
            productCount.setSiteId(site.getId());
            productCount.setSatellite(satellite);
            productCount.setStartDate(start.toLocalDate());
            productCount.setEndDate(end.toLocalDate());
            productCount.setCount((int) count);
            persistenceManager.save(productCount);
            final Future<List<EOProduct>> future = Config.getWorkerFor(queryConfiguration)
                    .submit(() -> downloadService.query(site.getId(), query, queryConfiguration, count));
            final List<EOProduct> results = future.get();
            logger.info(String.format(MESSAGE, site.getName(), satellite.name(),
                                      String.format("Query returned %s products", results.size())));
            String forceStartFlag = String.format(ConfigurationKeys.DOWNLOADER_SITE_FORCE_START_ENABLED, satellite.shortName());
            if (Config.getAsBoolean(site.getId(), forceStartFlag, false)) {
                // one-time forced lookup, therefore remove all products that have a NOK status
                logger.info(String.format("Forced lookup kicking in for site '%s' and satellite '%s', will delete products with status in (1, 3, 4)",
                                          site.getShortName(), satellite.shortName()));
                List<DownloadProduct> failed = persistenceManager.getProducts(site.getId(), satellite.value(),
                                                                              Status.DOWNLOADING, Status.FAILED,
                                                                              Status.ABORTED);
                int deleted = persistenceManager.deleteProducts(failed);
                if (deleted > 0) {
                    logger.info(String.format("Forced lookup purged %s products for site '%s' and satellite '%s'",
                                              deleted, site.getShortName(), satellite.shortName()));
                }
                Config.setSetting(site.getId(), forceStartFlag, "false");
                logger.info(String.format("Flag '%s' for site '%s' was reset. Next lookup will perform normally",
                                          forceStartFlag, satellite.shortName()));
            }
            // Check for already downloaded products with status (2, 5, 6, 7).
            // If such products exist for other sites, they will be "duplicated" for the current site
            String setting = Config.getSetting(ConfigurationKeys.SKIP_EXISTING_PRODUCTS, null);
            if (setting == null) {
                setting = "false";
                Config.setSetting(ConfigurationKeys.SKIP_EXISTING_PRODUCTS, setting);
            }
            if (Boolean.parseBoolean(setting) && results.size() > 0) {
                List<DownloadProduct> withoutOrbitDirection = persistenceManager.getProductsWithoutOrbitDirection(site.getId(), Satellite.Sentinel1.value());
                if (withoutOrbitDirection != null && withoutOrbitDirection.size() > 0) {
                    for (DownloadProduct product : withoutOrbitDirection) {
                        EOProduct found = results.stream()
                                .filter(r -> r.getName().equals(product.getProductName().replace(".SAFE", "")))
                                .findFirst().orElse(null);
                        if (found != null && found.getAttributeValue("orbitdirection") != null) {
                            product.setOrbitType(OrbitType.valueOf(found.getAttributeValue("orbitdirection")));
                            persistenceManager.save(product);
                        }
                    }
                    withoutOrbitDirection.clear();
                }
                final List<String> existing = persistenceManager.getOtherSitesProducts(site.getId(),
                                                                                      results.stream()
                                                                                             .map(EOData::getName)
                                                                                             .collect(Collectors.toSet()));
                if (existing != null && existing.size() > 0) {
                    logger.info(String.format("The following products have already been downloaded for other sites: %s",
                                              String.join(",", existing)));
                    results.removeIf(r -> existing.contains(r.getName()));
                    persistenceManager.attachToSite(site, existing);
                }
            }
            if (results.size() > 0) {
                runningJobs.put(key, results.size());
                for (int i = 0; i < results.size(); i++) {
                    final List<EOProduct> subList = results.subList(i, i + 1);
                    Config.getWorkerFor(downloadConfiguration)
                            .submit(new DownloadTask(site, satellite, subList,
                                                     () -> downloadService.download(site.getId(),
                                                                                    subList,
                                                                                    tiles, path, downloadConfiguration),
                                                     LookupJob.this::downloadCompleted));
                }
            } else {
                cleanupJob(site, satellite);
            }
        } catch (Exception e) {
            e.printStackTrace();
            logger.severe(e.getMessage() + " @ " + String.join(" < ",
                                      Arrays.stream(e.getStackTrace())
                                              .map(StackTraceElement::toString)
                                              .collect(Collectors.toSet())));
            cleanupJob(site, satellite);
        }
    }

    private void cleanupJob(Site site, Satellite sat) {
        synchronized (runningJobs) {
            Tuple<String, String> key = new Tuple<>(site.getName(), sat.shortName());
            runningJobs.remove(key);
        }
    }

    private void downloadCompleted(Triple<String, String, String> key) {
        synchronized (runningJobs) {
            Tuple<String, String> jobKey = new Tuple<>(key.getKeyOne(), key.getKeyTwo());
            int current = runningJobs.get(jobKey) - 1;
            logger.fine(String.format("Download Completed [%s] - remaining products to complete for site '%s' and satellite '%s' = '%s'",
                    key.getKeyThree(), key.getKeyOne(), key.getKeyTwo(), current));
            if (current > 0) {
                runningJobs.put(jobKey, current);
            } else {
                runningJobs.remove(jobKey);
            }
        }
    }

    private class DownloadTask implements Runnable {

        private final Site site;
        private final Satellite satellite;
        private final Runnable runnable;
        private final List<EOProduct> products;
        private final Consumer<Triple<String, String, String>> callback;

        DownloadTask(Site site, Satellite sat, List<EOProduct> products, Runnable runnable, Consumer<Triple<String, String, String>> callback) {
            this.site = site;
            this.satellite = sat;
            this.products = products;
            this.runnable = runnable;
            this.callback = callback;
        }

        @Override
        public void run() {
            try {
                this.runnable.run();
            } catch (Exception ex) {
                logger.warning(ex.getMessage());
            } finally {
                Triple<String, String, String> key = new Triple<>(site.getName(),
                                                                satellite.shortName(),
                                                                String.join(",", products.stream().map(EOData::getName).collect(Collectors.toList())));
                this.callback.accept(key);
            }
        }
    }
}
