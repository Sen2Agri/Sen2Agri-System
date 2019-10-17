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
package org.esa.sen2agri.services.internal;

import org.esa.sen2agri.commons.*;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.ProductCount;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.services.DownloadService;
import org.esa.sen2agri.services.SensorProgress;
import org.esa.sen2agri.services.SiteHelper;
import org.esa.sen2agri.web.beans.Query;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import ro.cs.tao.EnumUtils;
import ro.cs.tao.datasource.*;
import ro.cs.tao.datasource.converters.ConversionException;
import ro.cs.tao.datasource.param.CommonParameterNames;
import ro.cs.tao.datasource.param.DataSourceParameter;
import ro.cs.tao.datasource.param.QueryParameter;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.messaging.Message;
import ro.cs.tao.messaging.Messaging;
import ro.cs.tao.messaging.Notifiable;
import ro.cs.tao.messaging.progress.*;
import ro.cs.tao.security.SystemPrincipal;
import ro.cs.tao.utils.Tuple;

import java.lang.reflect.Array;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.Instant;
import java.util.*;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Service("downloadService")
public class DownloadServiceImpl extends Notifiable implements DownloadService {

    private static final Map<String, TaskProgress> downloadsInProgress = Collections.synchronizedMap(new LinkedHashMap<>());
    //private static final Map<String, Key<Site, Satellite>> infoCache = Collections.synchronizedMap(new HashMap<>());
    private static final Map<Short, Long> estimatedProductCount = Collections.synchronizedMap(new HashMap<>());
    private final Map<Tuple<Short, Satellite>, List<DataSourceComponent>> querySiteDataSources;
    private final Map<Tuple<Short, Satellite>, List<DataSourceComponent>> dwnSiteDataSources;
    private ProductStatusListener productStatusListener;
    private static final int DEFAULT_PRODUCTS_PER_PAGE_NO = 50;

    @Autowired
    private PersistenceManager persistenceManager;
    @Autowired
    private SiteHelper siteHelper;

    public DownloadServiceImpl() {
        querySiteDataSources = new HashMap<>();
        dwnSiteDataSources = new HashMap<>();
        Messaging.subscribe(this, DataSourceTopics.PRODUCT_PROGRESS);
    }

    @Override
    public void setProductStatusListener(ProductStatusListener productStatusListener) {
        this.productStatusListener = productStatusListener;
    }

    @Override
    public long count(short siteId, Query queryObject, DataSourceConfiguration configuration) throws ParseException {
        long count = 0;
        if (queryObject != null) {
            final Satellite satellite = configuration.getSatellite();
            final DataSourceComponent dataSourceComponent = getDataSourceComponent(siteId, queryObject.getUser(), queryObject.getPassword(),
                    configuration, false);
            if (dataSourceComponent == null) {
                return count;
            }
            List<Query> subQueries = null;
            try {
                subQueries = queryObject.splitByParameter(CommonParameterNames.TILE);
            } catch (ConversionException e) {
                logger.warning("Cannot create subqueries. Reason: " + e.getMessage());
            }
            if (subQueries == null) {
                subQueries = new ArrayList<>();
                subQueries.add(queryObject);
            }
            final Map<String, DataSourceParameter> parameterDescriptorMap =
                    DataSourceManager.getInstance().getSupportedParameters(satellite.name(),
                                                                           configuration.getDataSourceName());
            for (Query subQuery : subQueries) {
                DataQuery query = dataSourceComponent.createQuery();
                Map<String, Object> paramValues = subQuery.getValues();
                for (Map.Entry<String, Object> entry : paramValues.entrySet()) {
                    final DataSourceParameter descriptor = parameterDescriptorMap.get(entry.getKey());
                    if (descriptor == null) {
                        throw new QueryException(String.format("Parameter [%s] not supported by data source '%s' for sensor '%s'",
                                                               entry.getKey(),
                                                               configuration.getDataSourceName(),
                                                               configuration.getSatellite().name()));
                    }
                    final Class type = descriptor.getType();
                    Object value = entry.getValue();
                    final QueryParameter queryParameter;
                    if (value != null && value.getClass().isArray()) {
                        queryParameter = query.createParameter(entry.getKey(),
                                                               type,
                                                               Date.class.isAssignableFrom(type) ?
                                                                       new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(Array.get(value, 0)))
                                                                       : Array.get(value, 0),
                                                               Date.class.isAssignableFrom(type) ?
                                                                       new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(Array.get(value, 1)))
                                                                       : Array.get(value, 1));
                    } else {
                        queryParameter = query.createParameter(entry.getKey(),
                                                               type,
                                                               Date.class.isAssignableFrom(type) ?
                                                                       new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(entry.getValue()))
                                                                       : entry.getValue());
                    }
                    query.addParameter(queryParameter);
                }
                count += query.getCount();
            }
            estimatedProductCount.put(siteId, count);
        }
        return count;
    }

    @Override
    public long getCount(short siteId) {
        return estimatedProductCount.containsKey(siteId) ? estimatedProductCount.get(siteId) : 0;
    }

    @Override
    public List<SensorProgress> getProgress(short siteId, short satelliteId) {
        Satellite satellite = new SatelliteConverter().convertToEntityAttribute(satelliteId);
        List<SensorProgress> progress = new ArrayList<>();
        List<ProductCount> productCounts = persistenceManager.countEstimated(siteId, satellite);
        if (productCounts != null) {
            for (ProductCount productCount : productCounts) {
                SensorProgress sensorProgress = new SensorProgress();
                sensorProgress.setDownloaded(persistenceManager.countCompleted(siteId, satellite,
                        productCount.getStartDate(), productCount.getEndDate()));
                sensorProgress.setEstimated(productCount.getCount());
                sensorProgress.setBegin(productCount.getStartDate());
                sensorProgress.setEnd(productCount.getEndDate());
                progress.add(sensorProgress);
            }
        }
        return progress;
    }

    @Override
    public List<EOProduct> query(short siteId, Query queryObject,
                                 DataSourceConfiguration configuration) throws ParseException {
        List<EOProduct> results = null;
        if (queryObject != null) {
            final DataSourceComponent dataSourceComponent = getDataSourceComponent(siteId, queryObject.getUser(), queryObject.getPassword(), configuration, false);
            if (dataSourceComponent == null) {
                return null;
            }
            final Satellite satellite = configuration.getSatellite();
            try {
                final Map<String, DataSourceParameter> parameterDescriptorMap =
                        DataSourceManager.getInstance().getSupportedParameters(satellite.name(),
                                configuration.getDataSourceName());
                List<Query> subQueries = null;
                try {
                    subQueries = queryObject.splitByParameter(CommonParameterNames.TILE);
                } catch (ConversionException e) {
                    logger.warning("Cannot create subqueries. Reason: " + e.getMessage());
                }
                if (subQueries == null) {
                    subQueries = new ArrayList<>();
                    subQueries.add(queryObject);
                }
                results = new ArrayList<>();
                int queryIdx = 1;
                for (Query subQuery : subQueries) {
                    DataQuery query = dataSourceComponent.createQuery();
                    query.setPageSize(DEFAULT_PRODUCTS_PER_PAGE_NO);
                    Map<String, Object> paramValues = subQuery.getValues();
                    for (Map.Entry<String, Object> entry : paramValues.entrySet()) {
                        final DataSourceParameter descriptor = parameterDescriptorMap.get(entry.getKey());
                        if (descriptor == null) {
                            throw new QueryException(String.format("Parameter [%s] not supported by data source '%s' for sensor '%s'",
                                                                   entry.getKey(),
                                                                   configuration.getDataSourceName(),
                                                                   configuration.getSatellite().name()));
                        }
                        final Class type = descriptor.getType();
                        Object value = entry.getValue();
                        final QueryParameter queryParameter;
                        if (value != null && value.getClass().isArray()) {
                            queryParameter = query.createParameter(entry.getKey(),
                                                                   type,
                                                                   Date.class.isAssignableFrom(type) ?
                                                                           new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(Array.get(value, 0)))
                                                                           : Array.get(value, 0),
                                                                   Date.class.isAssignableFrom(type) ?
                                                                           new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(Array.get(value, 1)))
                                                                           : Array.get(value, 1));
                        } else {
                            queryParameter = query.createParameter(entry.getKey(),
                                                                   type,
                                                                   Date.class.isAssignableFrom(type) ?
                                                                           new SimpleDateFormat(Constants.FULL_DATE_FORMAT).parse(String.valueOf(entry.getValue()))
                                                                           : entry.getValue());
                        }
                        query.addParameter(queryParameter);
                    }
                    int page = 1;
                    int currentCount;
                    do {
                        query.setPageNumber(page);
                        logger.fine(String.format("Querying page #%d (query %d of %d) for {site id=%d,satellite=%s}",
                                                  page, queryIdx, subQueries.size(), siteId, satellite.friendlyName()));
                        List<EOProduct> products = query.execute();
                        logger.fine(String.format("Page #%d (query %d of %d) for {site id=%d,satellite=%s} returned %d results",
                                                  page, queryIdx, subQueries.size(), siteId, satellite.friendlyName(), products.size()));
                        results.addAll(products);
                        currentCount = products.size();
                        page++;
                    } while (currentCount > 0);
                    queryIdx++;
                }
                results.sort(Comparator.comparing(EOProduct::getAcquisitionDate));
            } finally {
                querySiteDataSources.get(new Tuple<>(siteId, configuration.getSatellite())).remove(dataSourceComponent);
            }
        }
        return results;
    }

    @Override
    public List<EOProduct> download(short siteId, List<EOProduct> products, Set<String> tiles, String targetPath,
                                    DataSourceConfiguration configuration) {
        if (configuration == null) {
            throw new RuntimeException("Invalid datasource configuration");
        }
        final DataSourceComponent dataSourceComponent = getDataSourceComponent(siteId,
                                                                               configuration.getUser(),
                                                                               configuration.getPassword(),
                                                                               configuration, true);
        if (dataSourceComponent == null) {
            throw new RuntimeException("Cannot create the datasource component for configuration: " + configuration.toString());
        }
        try {
            products.forEach(p -> p.addAttribute("site", String.valueOf(siteId)));
            return dataSourceComponent.doFetch(products, tiles, targetPath,
                                               configuration.getLocalArchivePath(),
                                               configuration.getAdditionalSettings());
        } finally {
            if (products != null) {
                products.forEach(p -> downloadsInProgress.remove(p.getName()));
            }
            dwnSiteDataSources.get(new Tuple<>(siteId, configuration.getSatellite())).remove(dataSourceComponent);
        }
    }

    @Override
    public List<TaskProgress> getDownloadsInProgress(short siteId) {
        List<TaskProgress> tasks = new ArrayList<>();
        if (siteId > 0) {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> siteId == e.getKey().getKeyOne())
                    .map(Map.Entry::getValue).forEach(ldsc ->
                        ldsc.forEach(dsc -> {
                            if (dsc != null) {
                                final EOProduct product = dsc.getCurrentProduct();
                                if (product != null) {
                                    tasks.add(downloadsInProgress.get(product.getName()));
                                }
                            }
                        }));
        } else {
            tasks.addAll(downloadsInProgress.values());
        }
        return tasks;
    }

    @Override
    public void stop(short siteId) {
        if (siteId > 0) {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> siteId == e.getKey().getKeyOne())
                    .map(Map.Entry::getValue).forEach(ldsc ->
                        ldsc.forEach(dsc -> {
                          if (dsc != null) {
                              dsc.cancel();
                              final EOProduct product = dsc.getCurrentProduct();
                              if (product != null) {
                                  downloadsInProgress.remove(product.getName());
                              }

                          }
                      }));
            Config.setSetting(siteId, ConfigurationKeys.DOWNLOADER_ENABLED, "false");
        } else {
            dwnSiteDataSources.values().forEach(ldsc -> ldsc.forEach(DataSourceComponent::cancel));
            Config.setSetting((short) 0, ConfigurationKeys.DOWNLOADER_ENABLED, "false");
            downloadsInProgress.clear();
        }
    }

    @Override
    public void stop(short siteId, short satelliteId) {
        Satellite satellite = new SatelliteConverter().convertToEntityAttribute(satelliteId);
        if (siteId > 0) {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> siteId == e.getKey().getKeyOne() && satellite.equals(e.getKey().getKeyTwo()))
                    .map(Map.Entry::getValue).forEach(ldsc ->
                        ldsc.forEach(dsc -> {
                          if (dsc != null) {
                              dsc.cancel();
                              final EOProduct product = dsc.getCurrentProduct();
                              if (product != null) {
                                  downloadsInProgress.remove(product.getName());
                              }
                          }
                      }));
        } else {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> satellite.equals(e.getKey().getKeyTwo()))
                    .map(Map.Entry::getValue).forEach(ldsc ->
                        ldsc.forEach(dsc -> {
                              if (dsc != null) {
                                  dsc.cancel();
                                  final EOProduct product = dsc.getCurrentProduct();
                                  if (product != null) {
                                      downloadsInProgress.remove(product.getName());
                                  }

                              }
                          }));
        }
        //dwnSiteDataSources.entrySet().removeIf(e -> siteId == e.getKey().getKeyOne() && satellite.equals(e.getKey().getKeyTwo()));
        Config.setSetting(siteId,
                          String.format(ConfigurationKeys.DOWNLOADER_SENSOR_ENABLED,
                                  satellite.friendlyName().toLowerCase()),
                          "false");
    }

    @Override
    public void forceStart(String job, short siteId) {
        if (job == null || job.isEmpty()) {
            throw new IllegalArgumentException("Parameter [job] cannot be empty");
        }
        if (siteId == 0) {
            throw new IllegalArgumentException("Parameter [siteId] cannot be zero");
        }
        final List<Satellite> satellites = persistenceManager.getDataSourceConfigurations().stream()
                .map(DataSourceConfiguration::getSatellite).distinct().collect(Collectors.toList());
        boolean canForce = false;
        for (Satellite satellite : satellites) {
            canForce |= Config.getAsBoolean(siteId, String.format(ConfigurationKeys.DOWNLOADER_SENSOR_FORCE_START,
                                                                  satellite.friendlyName()), false);
        }
        if (!canForce) {
            return;
        }
        dwnSiteDataSources.entrySet().stream()
                .filter(e -> siteId == e.getKey().getKeyOne())
                .map(Map.Entry::getValue).forEach(ldsc ->
                                                          ldsc.forEach(dsc -> {
                                                              if (dsc != null) {
                                                                  dsc.cancel();
                                                                  final EOProduct product = dsc.getCurrentProduct();
                                                                  if (product != null) {
                                                                      downloadsInProgress.remove(product.getName());
                                                                  }

                                                              }
                                                          }));
        for (Satellite satellite : satellites) {
            Config.setSetting(siteId, String.format(ConfigurationKeys.DOWNLOADER_SENSOR_FORCE_START,
                                                    satellite.friendlyName()), "true");
        }
        start(siteId);
        sendCommand(Commands.DOWNLOADER_FORCE_START, job.toLowerCase(), siteId, null);
    }

    @Override
    public void forceStart(String job, short siteId, short satelliteId) {
        if (siteId == 0) {
            throw new IllegalArgumentException("Parameter [siteId] cannot be zero");
        }
        final Satellite satellite = EnumUtils.getEnumConstantByValue(Satellite.class, satelliteId);
        if (satellite == null) {
            throw new IllegalArgumentException("Invalid value for parameter [satelliteId]");
        }
        String configKey = String.format(ConfigurationKeys.DOWNLOADER_SENSOR_FORCE_START, satellite.friendlyName());
        if (Config.getAsBoolean(siteId, configKey, false)) {
            return;
        }
        dwnSiteDataSources.entrySet().stream()
                .filter(e -> siteId == e.getKey().getKeyOne() && satellite.equals(e.getKey().getKeyTwo()))
                .map(Map.Entry::getValue).forEach(ldsc ->
                                                          ldsc.forEach(dsc -> {
                                                              if (dsc != null) {
                                                                  dsc.cancel();
                                                                  final EOProduct product = dsc.getCurrentProduct();
                                                                  if (product != null) {
                                                                      downloadsInProgress.remove(product.getName());
                                                                  }
                                                              }
                                                          }));
        Config.setSetting(siteId, configKey, "true");
        start(siteId, satelliteId);
        sendCommand(Commands.DOWNLOADER_FORCE_START, job, siteId, (int) satelliteId);
    }

    @Override
    public void start(short siteId) {
        if (siteId > 0) {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> siteId == e.getKey().getKeyOne())
                    .map(Map.Entry::getValue).forEach(ldsc ->
                        ldsc.forEach(dsc -> {
                        if (dsc != null) {
                            dsc.resume();
                        }
                    }));
        } else {
            dwnSiteDataSources.values().forEach(ldsc -> ldsc.forEach(DataSourceComponent::resume));
        }
        Config.setSetting(siteId, ConfigurationKeys.DOWNLOADER_ENABLED, "true");
    }

    @Override
    public void start(short siteId, short satelliteId) {
        Satellite satellite = EnumUtils.getEnumConstantByValue(Satellite.class, satelliteId);
        if (siteId > 0) {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> siteId == e.getKey().getKeyOne() && satellite.equals(e.getKey().getKeyTwo()))
                    .map(Map.Entry::getValue).forEach(ldsc -> ldsc.forEach(dsc -> {
                        if (dsc != null) {
                            dsc.resume();
                        }
                    })
            );
        } else {
            dwnSiteDataSources.entrySet().stream()
                    .filter(e -> satellite.equals(e.getKey().getKeyTwo()))
                    .map(Map.Entry::getValue).forEach(ldsc -> ldsc.forEach(dsc -> {
                        if (dsc != null) {
                            dsc.resume();
                        }
                    })
            );
        }
        Config.setSetting(siteId,
                          String.format(ConfigurationKeys.DOWNLOADER_SENSOR_ENABLED,
                                  satellite.friendlyName().toLowerCase()),
                          "true");
    }

    @Override
    protected void onMessageReceived(Message data) {
        String contents = data.getPayload();
        String taskName;
        Site site = null;
        Satellite satellite = null;
        Tuple<Site, Satellite> productInfo;
        if (data instanceof ActivityStartMessage) {
            taskName = ((ActivityStartMessage) data).getTaskName();
            if ((productInfo = getProductInfo(taskName)) != null) {
                site = productInfo.getKeyOne();
                satellite = productInfo.getKeyTwo();
            }
            downloadsInProgress.put(taskName,
                                    new TaskProgress(taskName, site, satellite, 0));
        } else if (data instanceof ActivityEndMessage) {
            downloadsInProgress.remove(((ActivityEndMessage) data).getTaskName());
        } else if (data instanceof SubActivityStartMessage) {
            taskName = ((SubActivityStartMessage) data).getTaskName();
            if (downloadsInProgress.containsKey(taskName)) {
                double mainProgress = downloadsInProgress.get(taskName).getProgress();
                if ((productInfo = getProductInfo(taskName)) != null) {
                    site = productInfo.getKeyOne();
                    satellite = productInfo.getKeyTwo();
                }
                downloadsInProgress.put(taskName,
                                        new TaskProgress(taskName, site, satellite, mainProgress));
            }
        } else if (data instanceof SubActivityEndMessage) {
            final String mainTask = ((SubActivityEndMessage) data).getTaskName();
            final TaskProgress taskProgress = downloadsInProgress.get(mainTask);
            if (taskProgress != null) {
                double mainProgress = taskProgress.getProgress();
                if ((productInfo = getProductInfo(mainTask)) != null) {
                    site = productInfo.getKeyOne();
                    satellite = productInfo.getKeyTwo();
                }
                downloadsInProgress.put(mainTask,
                                        new TaskProgress(mainTask, site, satellite, mainProgress));
            }
        } else if (data instanceof SubActivityProgressMessage) {
            SubActivityProgressMessage casted = (SubActivityProgressMessage) data;
            final String mainTask = casted.getTaskName();
            if ((productInfo = getProductInfo(mainTask)) != null) {
                site = productInfo.getKeyOne();
                satellite = productInfo.getKeyTwo();
            }
            downloadsInProgress.put(mainTask, new TaskProgress(mainTask, site, satellite, casted.getTaskProgress()));
        } else if (data instanceof ActivityProgressMessage) {
            ActivityProgressMessage casted = (ActivityProgressMessage) data;
            final String mainTask = casted.getTaskName();
            if ((productInfo = getProductInfo(mainTask)) != null) {
                site = productInfo.getKeyOne();
                satellite = productInfo.getKeyTwo();
            }
            downloadsInProgress.put(mainTask, new TaskProgress(mainTask, site, satellite, casted.getProgress()));
        }

    }

    private void sendCommand(String name, String job, int siteId, Integer satelliteId) {
        Message message = new Message();
        message.setUser(SystemPrincipal.instance().getName());
        message.setTopic(Topics.COMMAND);
        message.setTimestamp(Instant.now().toEpochMilli());
        message.setMessage(name);
        message.addItem("job", job);
        message.addItem("siteId", String.valueOf(siteId));
        if (satelliteId != null) {
            message.addItem("satelliteId", String.valueOf(satelliteId.intValue()));
        }
        Messaging.send(SystemPrincipal.instance(), Topics.COMMAND, message);
    }

    private Tuple<Site, Satellite> getProductInfo(String productName) {
        //Key<Site, Satellite> info = infoCache.get(productName);
        Tuple<Site, Satellite> info = null;
        //if (info == null) {
            for (Map.Entry<Tuple<Short, Satellite>, List<DataSourceComponent>> entry : dwnSiteDataSources.entrySet()) {
                for (DataSourceComponent component : entry.getValue()) {
                    EOProduct currentProduct = component.getCurrentProduct();
                    if (currentProduct != null && currentProduct.getName().equals(productName)) {
                        Tuple<Short, Satellite> key = entry.getKey();
                        Site site = persistenceManager.getSiteById(key.getKeyOne());
                        if (site != null) {
                            info = new Tuple<>(site, key.getKeyTwo());
                        }
                        break;
                    }
                }
            }
        //}
        return info;
    }

    private DataSourceComponent getDataSourceComponent(short siteId, String user, String password, DataSourceConfiguration configuration, boolean isDwn) {
        final Satellite satellite = configuration.getSatellite();
        boolean dwnEnabled = Config.getAsBoolean(siteId,
                String.format(ConfigurationKeys.DOWNLOADER_SENSOR_ENABLED,
                        satellite.friendlyName().toLowerCase()),
                true);
        if (!dwnEnabled) {
            return null;
        }

        final Map<Tuple<Short, Satellite>, List<DataSourceComponent>> siteDataSources = isDwn ? dwnSiteDataSources : querySiteDataSources;
        synchronized (this.dwnSiteDataSources) {
            Tuple<Short, Satellite> key = new Tuple<>(siteId, satellite);
            siteDataSources.computeIfAbsent(key, k -> new ArrayList<>());
            List<DataSourceComponent> dataSourceComponents = siteDataSources.get(key);
            DataSourceComponent  dataSourceComponent = dataSourceComponents.stream()
                                                            .filter(c -> c.getSensorName().equals(satellite.name()) &&
                                                                         c.getDataSourceName().equals(configuration.getDataSourceName()) &&
                                                                         (user == null ? c.getUserName() == null : user.equals(c.getUserName())))
                                                            .findFirst().orElse(null);
            if (dataSourceComponent == null) {
                dataSourceComponent = new DataSourceComponent(satellite.name(), configuration.getDataSourceName());
                siteDataSources.get(key).add(dataSourceComponent);
                dataSourceComponent.setUserCredentials(user, password);
                if (isDwn) {
                    dataSourceComponent.setProductStatusListener(this.productStatusListener);
                    dataSourceComponent.setFetchMode(configuration.getFetchMode());
                    dataSourceComponent.setMaxRetries(configuration.getMaxRetries());
                }
            }
            return dataSourceComponent;
        }
    }
}
