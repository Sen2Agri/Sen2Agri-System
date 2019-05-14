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
package org.esa.sen2agri.commons;

import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.*;
import org.esa.sen2agri.services.DownloadService;
import ro.cs.tao.EnumUtils;
import ro.cs.tao.datasource.DataSource;
import ro.cs.tao.datasource.DataSourceManager;
import ro.cs.tao.datasource.remote.FetchMode;
import ro.cs.tao.datasource.util.NetUtils;
import ro.cs.tao.utils.StringUtilities;
import ro.cs.tao.utils.executors.NamedThreadPoolExecutor;

import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.logging.Logger;

/**
 * @author Cosmin Cara
 */
public class Config {

    private static final List<DataSourceConfiguration> dataSourceConfigurations = new ArrayList<>();
    private static final Map<DataSourceConfiguration, ThreadPoolExecutor> dataSourceExecutors = Collections.synchronizedMap(new HashMap<>());
    private static final ExecutorService backgroundWorker = Executors.newSingleThreadExecutor();
    private static PersistenceManager persistenceManager;
    private static DownloadService downloadService;
    private static Properties configuration;

    public static Properties getFileConfiguration() { return configuration; }
    public static void setFileConfiguration(Properties properties) { configuration = properties; }

    public static void setPersistenceManager(PersistenceManager manager) {
        if (persistenceManager == null || !persistenceManager.equals(manager)) {
            persistenceManager = manager;
            reload();
        }
    }

    public static void setDownloadService(DownloadService downloadService) {
        Config.downloadService = downloadService;
    }

    public static PersistenceManager getPersistenceManager() {
        return persistenceManager;
    }

    public static DownloadService getDownloadService() {
        return downloadService;
    }

    public static void reload() {
        dataSourceConfigurations.clear();
        dataSourceConfigurations.addAll(persistenceManager.getDataSourceConfigurations());
        DataSourceManager dataSourceManager= DataSourceManager.getInstance();
        final List<DataSourceConfiguration> toRemove = new ArrayList<>();
        for (DataSourceConfiguration ds : dataSourceConfigurations) {
            if (configuration != null) {
                boolean changed = false;
                final DataSource dataSource = dataSourceManager.get(ds.getSatellite().name(), ds.getDataSourceName());
                if (dataSource == null) {
                    toRemove.add(ds);
                    continue;
                }
                final String simpleName = dataSource.getClass().getSimpleName();
                if (StringUtilities.isNullOrEmpty(ds.getUser())) {
                    // Only set credentials from properties if not set in the database
                    String user = configuration.getProperty(simpleName + ".username", null);
                    String password = configuration.getProperty(simpleName + ".password", null);
                    if (user != null && password != null) {
                        ds.setUser(user.trim());
                        ds.setPassword(password.trim());
                        changed = true;
                    }
                }
                String dsKey = simpleName + "." + ds.getSatellite().name();
                if (ds.getScope() == 0) {
                    // Only set scope from properties if not set in the database
                    int scope = Integer.parseInt(configuration.getProperty(dsKey + ".scope", "0"));
                    if (scope != 0) {
                        ds.setScope(scope);
                        changed = true;
                    }
                }
                /*String enabled = configuration.getProperty(dsKey + ".enabled");
                if (enabled != null) {
                    ds.setEnabled(Boolean.parseBoolean(enabled));
                    changed = true;
                }*/
                if (ds.getLocalArchivePath() == null) {
                    // Only set local arhive path from properties if not set in the database
                    String localRoot = configuration.getProperty(dsKey + ".local_archive_path");
                    if (localRoot != null && !"".equals(localRoot.trim())) {
                        ds.setLocalArchivePath(localRoot.trim());
                        changed = true;
                    }
                }
                if (ds.getFetchMode() == null) {
                    // Only set fetch mode from properties if not set in the database
                    int fetchMode = Integer.parseInt(configuration.getProperty(dsKey + ".fetch_mode", "0"));
                    if (fetchMode != 0) {
                        ds.setFetchMode(EnumUtils.getEnumConstantByValue(FetchMode.class, fetchMode));
                        changed = true;
                    }
                }
                if (changed) {
                    persistenceManager.save(ds);
                }

                String port = configuration.getProperty("proxy.port");
                String host = configuration.getProperty("proxy.host");
                if (host != null && !host.isEmpty()) {
                    NetUtils.setProxy(configuration.getProperty("proxy.type"),
                            host.trim(),
                            port == null || port.isEmpty() ? 0 : Integer.parseInt(port),
                            configuration.getProperty("proxy.user"),
                            configuration.getProperty("proxy.password"));
                }
                Properties properties = new Properties();
                configuration.stringPropertyNames()
                        .stream()
                        .filter(n -> n.startsWith(dsKey))
                        .forEach(n -> properties.put(n.replace(dsKey + ".", ""), configuration.get(n)));
                ds.setAdditionalSettings(properties);
            }
            final int maxConnections = ds.getMaxConnections();
            if (dataSourceExecutors.containsKey(ds)) {
                final ThreadPoolExecutor executor = dataSourceExecutors.get(ds);
                if (executor != null && !executor.isShutdown() && !executor.isTerminated()) {
                    if (executor.getMaximumPoolSize() != maxConnections) {
                        backgroundWorker.submit(() -> {
                            executor.shutdownNow();
                            dataSourceExecutors.put(ds,
                                                    new NamedThreadPoolExecutor(ds.getDataSourceName() + "-" +
                                                                                        ds.getSatellite().shortName(),
                                                                                maxConnections));
                        });
                    }
                }
            } else {
                dataSourceExecutors.put(ds,
                                        new NamedThreadPoolExecutor(ds.getDataSourceName() + "-" +
                                                                            ds.getSatellite().shortName(),
                                                                    maxConnections));
            }
        }
        for (DataSourceConfiguration ds : toRemove) {
            persistenceManager.remove(ds);
        }
    }

    public static ThreadPoolExecutor getWorkerFor(DataSourceConfiguration dataSourceConfiguration) {
        return dataSourceExecutors.get(dataSourceConfiguration);
    }

    public static String getProperty(String name) {
        return getProperty(name, null);
    }
    public static String getProperty(String name, String defaultValue) {
        if (configuration == null) {
            configuration = new Properties();
        }
        return configuration.getProperty(name, defaultValue);
    }

    public static Map<String, String> getSiteSettings(short siteId) {
        Map<String, String> retMap = getSiteConfig().get(siteId);
        return (retMap == null) ? new HashMap<>() : retMap;
    }

    public static String getSetting(String name, String defaultValue) {
        final Optional<Map<String, String>> map = getSiteConfig().values().stream().filter(m -> m.containsKey(name)).findFirst();
        Optional<String> value = map.map(m -> m.get(name));
        if (value.isPresent()) {
            return value.orElse(null);
        } else {
            Logger.getLogger(Config.class.getName()).warning(String.format("Config key [%s] not found, using default value '%s'",
                                                                           name, defaultValue));
            persistenceManager.saveSetting((short) 0, name, defaultValue);
            return defaultValue;
        }
    }

    public static String getSetting(short siteId, String name, String defaultValue) {
        List<ConfigurationItem> items = persistenceManager.getConfigurationItem(name);
        ConfigurationItem item = null;
        if (items != null && items.size() > 0) {
            item = items.stream().filter(i -> (i.getSiteId() != null && i.getSiteId() == siteId))
                    .findFirst().orElse(null);
            if (item == null) {
                item = items.stream().filter(i -> i.getSiteId() == null)
                        .findFirst().orElse(null);
            }
        }
        return item != null ? item.getValue() : defaultValue;
    }

    /**
     * Returns the value from the site config or null if the key is not present
     * @param siteId the site id
     * @param name the key
     * @return the value or null if not present in site config
     */
    public static String getSetting(short siteId, String name) {
        return getSetting(siteId, name, null);
    }

    public static String[] getSettingValues(String name, String[] defaultValues) {
        String value = getSetting(name, null);
        return value != null ? value.split(";") : defaultValues;
    }

    public static Integer[] getSettingValues(String name, Integer[] defaultValues) {
        String[] values = getSettingValues(name, (String[]) null);
        return values != null ? Arrays.stream(values).map(Integer::parseInt).toArray(Integer[]::new) : defaultValues;
    }

    public static boolean getAsBoolean(String name, boolean defaultValue) {
        return Boolean.parseBoolean(getSetting(name, String.valueOf(defaultValue)));
    }

    public static boolean getAsBoolean(short siteId, String name, boolean defaultValue) {
        return Boolean.parseBoolean(getSetting(siteId, name, String.valueOf(defaultValue)));
    }

    /**
     * Returns the boolean value from the site config or null if the key is not present
     * @param siteId the site id
     * @param name the key
     * @return the boolean value or null if not present in site config
     */
    public static Boolean getAsBoolean(short siteId, String name) {
        String val = getSetting(siteId, name);
        return val != null ? Boolean.parseBoolean(val) : null;
    }

    public static int getAsInteger(String name, int defaultValue) {
        return Integer.parseInt(getSetting(name, String.valueOf(defaultValue)));
    }

    public static int getAsInteger(short siteId, String name, int defaultValue) {
        return Integer.parseInt(getSetting(siteId, name, String.valueOf(defaultValue)));
    }

    public static double getAsDouble(String name, double defaultValue) {
        return Double.parseDouble(getSetting(name, String.valueOf(defaultValue)));
    }

    public static double getAsDouble(short siteId, String name, double defaultValue) {
        return Double.parseDouble(getSetting(siteId, name, String.valueOf(defaultValue)));
    }

    public static Map<Satellite, List<DataSourceConfiguration>> getQueryConfigurations() {
        final Map<Satellite, List<DataSourceConfiguration>> configurations = new HashMap<>();
        dataSourceConfigurations.stream()
                .filter(ds -> (ds.isEnabled() && (ds.getScope() & Scope.QUERY) != 0))
                .forEach(ds -> {
                    Satellite satellite = ds.getSatellite();
                    if (!configurations.containsKey(satellite)) {
                        configurations.put(satellite, new ArrayList<>());
                    }
                    configurations.get(satellite).add(ds);
                });
        return configurations;
    }

    public static DataSourceConfiguration getQueryConfiguration(Site site, Satellite satellite) {
        return dataSourceConfigurations.stream()
                .filter(ds -> satellite.equals (ds.getSatellite()) &&
                        (ds.isEnabled() && (ds.getScope() & Scope.QUERY) != 0) &&
                        (ds.getSiteId() == null || ds.getSiteId().equals(site.getId())))
                .findFirst().orElse(null);
    }

    public static DataSourceConfiguration getDownloadConfiguration(Site site, Satellite satellite) {
        return dataSourceConfigurations.stream()
                .filter(ds -> satellite.equals (ds.getSatellite()) &&
                        (ds.isEnabled() && (ds.getScope() & Scope.DOWNLOAD) != 0) &&
                        (ds.getSiteId() == null || ds.getSiteId().equals(site.getId())))
                .findFirst().orElse(null);
    }

    public static Map<Satellite, List<DataSourceConfiguration>> getDownloadConfigurations() {
        final Map<Satellite, List<DataSourceConfiguration>> configurations = new HashMap<>();
        dataSourceConfigurations.stream()
                .filter(ds -> (ds.isEnabled() && (ds.getScope() & Scope.DOWNLOAD) != 0))
                .forEach(ds -> {
                    Satellite satellite = ds.getSatellite();
                    if (!configurations.containsKey(satellite)) {
                        configurations.put(satellite, new ArrayList<>());
                    }
                    configurations.get(satellite).add(ds);
                });
        return configurations;
    }

    public static void setSetting(String name, String value) {
        setSetting((short) 0, name, value);
    }

    public static void setSetting(short siteId, String name, String value) {
        /*final Map<String, String> commonMap = persistenceManager.getConfiguration().get(siteId);
        if (commonMap != null) {
            commonMap.put(name, value);
        }*/
        persistenceManager.saveSetting(siteId, name, value);
    }

    public static boolean isFeatureEnabled(short siteId, String configKey) {
        // check first if the feature is present for the site
        Boolean siteVal = getAsBoolean(siteId, configKey);
        if (siteVal != null) {
            return siteVal;
        }
        // if not present in site keys, then return the global value or false if not present also here
        return getAsBoolean((short) 0, configKey, false);
    }

    private static Map<Short, Map<String, String>> getSiteConfig() {
        return persistenceManager.getConfiguration();
    }
}
