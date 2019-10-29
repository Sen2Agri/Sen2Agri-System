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
import org.esa.sen2agri.commons.Commands;
import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.commons.Constants;
import org.esa.sen2agri.commons.ProcessingTopic;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.entities.*;
import org.esa.sen2agri.entities.enums.OrbitType;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.entities.enums.Status;
import org.esa.sen2agri.web.beans.Query;
import org.locationtech.jts.geom.Geometry;
import org.locationtech.jts.io.ParseException;
import org.locationtech.jts.io.WKTReader;
import org.quartz.JobDataMap;
import ro.cs.tao.EnumUtils;
import ro.cs.tao.datasource.param.CommonParameterNames;
import ro.cs.tao.datasource.remote.FetchMode;
import ro.cs.tao.datasource.util.TileExtent;
import ro.cs.tao.eodata.EOData;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.eodata.Polygon2D;
import ro.cs.tao.messaging.Message;
import ro.cs.tao.products.landsat.Landsat8TileExtent;
import ro.cs.tao.products.sentinels.Sentinel2TileExtent;
import ro.cs.tao.utils.Triple;
import ro.cs.tao.utils.Tuple;

import java.awt.geom.Path2D;
import java.nio.file.Paths;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
public class LookupJob extends DownloadJob {

    private static final Map<Tuple<String, String>, Integer> runningJobs = Collections.synchronizedMap(new HashMap<>());
    private static final Object sharedLock = new Object();
    public static final int DAYS_BACK = 6;

    public LookupJob() {
        super();
    }

    @Override
    public String configKey() {
        return ConfigurationKeys.SCHEDULED_LOOKUP_ENABLED;
    }

    @Override
    public String groupName() { return "Lookup"; }

    @Override
    protected void onMessageReceived(Message message) {
        String jobType = message.getItem("job");
        if (jobType != null && !jobType.equals(groupName().toLowerCase())) {
            return;
        }
        String command = message.getMessage();
        logger.fine(String.format("%s:%d received command [%s]",
                                  LookupJob.class.getSimpleName(), this.hashCode(), command));
        if (Commands.DOWNLOADER_FORCE_START.equals(command)) {
            String siteId = message.getItem("siteId");
            if (siteId != null) {
                Site site = persistenceManager.getSiteById(Short.parseShort(siteId));
                String satelliteId = message.getItem("satelliteId");
                if (satelliteId != null) {
                    Satellite satellite = EnumUtils.getEnumConstantByValue(Satellite.class, Short.parseShort(satelliteId));
                    if (satellite != null) {
                        Tuple<String, String> key = new Tuple<>(site.getName(), satellite.friendlyName());
                        runningJobs.remove(key);
                        logger.fine(String.format("Job with key %s removed", key));
                    }
                } else {
                    Set<Tuple<String, String>> keySet = runningJobs.keySet();
                    for (Tuple<String, String> key : keySet) {
                        if (key.getKeyOne().equals(site.getName())) {
                            runningJobs.remove(key);
                            logger.fine(String.format("Job with key %s removed", key));
                        }
                    }
                }
            }
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    protected void executeImpl(JobDataMap dataMap) {
        final Site site =  (Site) dataMap.get("site");
        if (!site.isEnabled()) {
            return;
        }
        final DataSourceConfiguration queryConfig = (DataSourceConfiguration) dataMap.get("queryConfig");
        final DataSourceConfiguration downloadConfig = (DataSourceConfiguration) dataMap.get("downloadConfig");
        final Satellite satellite = queryConfig.getSatellite();
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
        final short siteId = site.getId();
        final List<Season> seasons = persistenceManager.getEnabledSeasons(siteId);
        try {
            if (seasons != null && seasons.size() > 0) {
                seasons.sort(Comparator.comparing(Season::getStartDate));
                logger.fine(String.format(MESSAGE, site.getShortName(), satellite.name(),
                                          "Seasons defined: " +
                                                  seasons.stream()
                                                          .map(Season::toString)
                                                          .collect(Collectors.joining(";"))));
                final LocalDateTime startDate = getStartDate(satellite, site, seasons);
                final LocalDateTime endDate = getEndDate(seasons);
                logger.fine(String.format(MESSAGE, site.getShortName(), satellite.name(),
                                          String.format("Using start date: %s and end date: %s", startDate, endDate)));
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
                                                  "No products to download (endDate past startDate)"));
                    }
                } else {
                    logger.warning(String.format(MESSAGE, site.getName(), satellite.name(), "Season not started"));
                }
            } else {
                logger.warning(String.format(MESSAGE, site.getName(), satellite.name(),
                                             "No season defined"));
            }
        } catch (Throwable e) {
            cleanupJob(site, satellite);
        }
    }

    protected LocalDateTime getStartDate(Satellite satellite, Site site, List<Season> seasons) {
        LocalDateTime minStartDate = seasons.get(0).getStartDate().atStartOfDay();
        int downloaderStartOffset = Config.getAsInteger(ConfigurationKeys.DOWNLOADER_START_OFFSET, 0);
        if (satellite == Satellite.Sentinel1) {
            minStartDate = minStartDate.minusDays(DAYS_BACK);
        } else {
            if (downloaderStartOffset > 0) {
                minStartDate = minStartDate.minusMonths(downloaderStartOffset);
            }
        }
        LocalDateTime startDate;
        LocalDateTime lastDate = null;
        // Only use the last download date if the flag 'downloader.%s.forcestart' is set to 'false' or not present
        if (!Config.getAsBoolean(site.getId(),
                                 String.format(ConfigurationKeys.DOWNLOADER_SENSOR_FORCE_START, satellite.friendlyName()),
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
        final Query query = new Query();
        query.setUser(queryConfiguration.getUser());
        query.setPassword(queryConfiguration.getPassword());
        final Map<String, Object> params = new HashMap<>();
        final Satellite satellite = queryConfiguration.getSatellite();
        final Set<String> tiles = persistenceManager.getSiteTiles(site, downloadConfiguration.getSatellite());
        params.put(CommonParameterNames.START_DATE,
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        params.put(CommonParameterNames.END_DATE,
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        params.put(CommonParameterNames.FOOTPRINT, Polygon2D.fromWKT(site.getExtent()));
        if (tiles != null && tiles.size() > 0) {
            int initialSize = tiles.size();
            logger.finest(String.format("Validating %s tile filter for site %s", satellite.friendlyName(), site.getShortName()));
            validateTiles(site, tiles, satellite);
            logger.finest(String.format("%s %s tiles were discarded. Filter has %d tiles.",
                                        tiles.size() == initialSize ? "No" : initialSize - tiles.size(),
                                        satellite.friendlyName(),
                                        tiles.size()));
            final String tileList;
            if (tiles.size() == 1) {
                tileList = tiles.stream().findFirst().get();
            } else {
                tileList = "[" + String.join(",", tiles) + "]";
            }
            params.put(CommonParameterNames.TILE, tileList);
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
            final Tuple<String, String> key = new Tuple<>(site.getName(), satellite.friendlyName());
            runningJobs.put(key, 0);
            // Invoke the downloadService via this method to control the number of connections
            ThreadPoolExecutor worker = Config.getWorkerFor(queryConfiguration);
            /*final Future<Long> futureCount = worker.submit(() -> downloadService.count(site.getId(), query, queryConfiguration));
            long count = futureCount.get();
            logger.finest(String.format("Estimated number of records for site %s and satellite %s: %d",
                                        site.getShortName(), satellite.shortName(), count));
            final ProductCount productCount = new ProductCount();
            productCount.setSiteId(site.getId());
            productCount.setSatellite(satellite);
            productCount.setStartDate(start.toLocalDate());
            productCount.setEndDate(end.toLocalDate());
            productCount.setCount((int) count);
            persistenceManager.save(productCount);*/
            // Invoke the downloadService via this method to control the number of connections
            final Future<List<EOProduct>> future = worker.submit(() -> downloadService.query(site.getId(), query, queryConfiguration));
            final List<EOProduct> results = future.get();
            final ProductCount productCount = new ProductCount();
            productCount.setSiteId(site.getId());
            productCount.setSatellite(satellite);
            productCount.setStartDate(start.toLocalDate());
            productCount.setEndDate(end.toLocalDate());
            productCount.setCount((int) results.size());
            persistenceManager.save(productCount);
            logger.info(String.format(MESSAGE, site.getName(), satellite.name(),
                                      String.format("Found %d products for site %s and satellite %s",
                                                    results.size(), site.getShortName(), satellite.friendlyName())));
            final String forceStartKey = String.format(ConfigurationKeys.DOWNLOADER_SENSOR_FORCE_START, satellite.friendlyName());
            if (Config.getAsBoolean(site.getId(), forceStartKey, false)) {
                // one-time forced lookup, therefore remove all products that have a NOK status
                logger.info(String.format("Forced lookup kicking in for site '%s' and satellite '%s', will delete products with status in (1, 3, 4)",
                                          site.getShortName(), satellite.friendlyName()));
                List<DownloadProduct> failed = persistenceManager.getProducts(site.getId(), satellite.value(),
                                                                              Status.DOWNLOADING, Status.FAILED,
                                                                              Status.ABORTED);
                int deleted = persistenceManager.deleteProducts(failed);
                if (deleted > 0) {
                    logger.info(String.format("Forced lookup purged %s products for site '%s' and satellite '%s'",
                                              deleted, site.getShortName(), satellite.friendlyName()));
                }
                Config.setSetting(site.getId(), forceStartKey, "false");
                logger.info(String.format("Flag '%s' for site '%s' and satellite '%s' was reset. Next lookup will perform normally",
                                          forceStartKey, site.getShortName(), satellite.friendlyName()));
            }
            // Check for already downloaded products with status (2, 5, 6, 7).
            // If such products exist for other sites, they will be "duplicated" for the current site
            final boolean skipExisting = Boolean.parseBoolean(Config.getSetting(ConfigurationKeys.SKIP_EXISTING_PRODUCTS, "false"));
            if (skipExisting && results.size() > 0) {
                final List<DownloadProduct> withoutOrbitDirection = persistenceManager.getProductsWithoutOrbitDirection(site.getId(), Satellite.Sentinel1.value());
                if (withoutOrbitDirection != null && withoutOrbitDirection.size() > 0) {
                    logger.info(String.format("Found %d products in database without orbit direction. Attempting to set it.",
                                              withoutOrbitDirection.size()));
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
                    logger.info(String.format("The following products have already been downloaded for other sites and will not be re-downloaded: %s",
                                              String.join(",", existing)));
                    results.removeIf(r -> existing.contains(r.getName()));
                    persistenceManager.attachToSite(site, existing);
                }
            }
            final int resultsSize = results.size();
            logger.fine(String.format("Actual products to download for site %s and satellite %s: %d",
                                      site.getShortName(), satellite.friendlyName(), resultsSize));
            if (resultsSize > 0) {
                worker = Config.getWorkerFor(downloadConfiguration);
                runningJobs.put(key, resultsSize);
                for (int i = 0; i < resultsSize; i++) {
                    final List<EOProduct> subList = results.subList(i, i + 1);
                    worker.submit(
                            new DownloadTask(logger, site, satellite, subList,
                                             () -> {
                                                 Instant startTime = Instant.now();
                                                 downloadService.download(site.getId(), subList,
                                                                          tiles, path, downloadConfiguration);
                                                 long seconds = Duration.between(startTime, Instant.now()).getSeconds();
                                                 if (downloadConfiguration.getFetchMode() == FetchMode.SYMLINK && seconds > 10) {
                                                     sendNotification(ProcessingTopic.PROCESSING_ATTENTION.value(),
                                                                      String.format("Lookup site \"%s\"", site.getName()),
                                                                      String.format("Symlink creation took %d seconds", seconds));
                                                 }
                                             }, LookupJob.this::downloadCompleted));
                }
            }
        } catch (Throwable e) {
            final String message = ExceptionUtils.getStackTrace(e);
            logger.severe(message);
            sendNotification(ProcessingTopic.PROCESSING_ATTENTION.value(),
                             String.format("Lookup site \"%s\"", site.getName()),
                             message);
        }/* finally {
            cleanupJob(site, satellite);
        }*/
    }

    private void validateTiles(Site site, Set<String> tiles, Satellite satellite) {
        if (tiles == null || tiles.size() == 0) {
            return;
        }
        final TileExtent extentHelper;
        switch (satellite) {
            case Sentinel2:
                extentHelper = Sentinel2TileExtent.getInstance();
                break;
            case Landsat8:
                extentHelper = Landsat8TileExtent.getInstance();
                break;
            case Sentinel1:
            default:
                extentHelper = null;
                break;
        }
        if (extentHelper != null) {
            final WKTReader reader = new WKTReader();
            Geometry siteFootprint;
            try {
                siteFootprint = reader.read(site.getExtent());
            } catch (ParseException e) {
                logger.severe(String.format("Invalid geometry for site %s [%s]", site.getShortName(), site.getExtent()));
                return;
            }
            Iterator<String> iterator = tiles.iterator();
            while (iterator.hasNext()) {
                String tile = iterator.next();
                Path2D.Double tileExtent = extentHelper.getTileExtent(tile);
                if (tileExtent == null) {
                    logger.warning(String.format("No spatial footprint found for tile '%s'. Tile will be discarded.", tile));
                    iterator.remove();
                } else {
                    Polygon2D tilePolygon = Polygon2D.fromPath2D(tileExtent);
                    if (tilePolygon == null) {
                        logger.warning(String.format("Invalid spatial footprint found for tile '%s'. Tile will be discarded.", tile));
                        iterator.remove();
                    } else {
                        try {
                            Geometry tileFootprint = reader.read(tilePolygon.toWKT(8));
                            if (!siteFootprint.intersects(tileFootprint)) {
                                logger.warning(String.format("Tile '%s' does not intersect the footprint of site '%s'. Tile will be discarded.",
                                                             tile, site.getShortName()));
                                iterator.remove();
                            }
                        } catch (ParseException e) {
                            logger.severe(String.format("Invalid geometry for tile %s [%s]. Tile will be discarded", tile, e.getMessage()));
                            iterator.remove();
                        }
                    }
                }
            }
        }
    }

    private void cleanupJob(Site site, Satellite sat) {
        runningJobs.remove(new Tuple<>(site.getName(), sat.friendlyName()));
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
            logger.fine(String.format("Download completed [%s] - remaining products for {site:'%s',satellite:'%s'}: %d",
                    key.getKeyThree(), key.getKeyOne(), key.getKeyTwo(), current));
            if (current > 0) {
                runningJobs.put(jobKey, current);
            } else {
                runningJobs.remove(jobKey);
            }
        }
    }
}
