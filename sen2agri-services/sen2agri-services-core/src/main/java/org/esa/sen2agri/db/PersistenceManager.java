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
package org.esa.sen2agri.db;

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.entities.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.transaction.annotation.EnableTransactionManagement;
import org.springframework.transaction.annotation.Transactional;
import ro.cs.tao.datasource.DataSourceManager;
import ro.cs.tao.datasource.param.DataSourceParameter;

import javax.sql.DataSource;
import java.sql.SQLException;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.util.*;
import java.util.function.Predicate;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;

/**
 * @author Cosmin Cara
 */
@Configuration
@EnableTransactionManagement
@EnableJpaRepositories(basePackages = { "org.esa.sen2agri.db" })
@org.springframework.context.annotation.Scope("singleton")
public class PersistenceManager {

    @Autowired
    private DataSourceRepository dataSourceRepository;
    @Autowired
    private ConfigRepository configRepository;
    @Autowired
    private SiteRepository siteRepository;
    @Autowired
    private SeasonRepository seasonRepository;
    @Autowired
    private ProductTypeRepository productTypeRepository;
    @Autowired
    private ProcessorRepository processorRepository;

    private ProductRepository productRepository;
    private DownloadProductRepository downloadProductRepository;
    private DownloadProductTileRepository downloadProductTileRepository;
    private NonMappedEntitiesRepository nonMappedEntitiesRepository;

    /**
     * Instance of the DB configuration
     */
    @Autowired
    private DatabaseConfiguration dbConfig;

    //@Transactional
    public DownloadProduct save(DownloadProduct eoProduct) {
        return getDownloadProductRepository().save(eoProduct);
    }

    @Transactional
    public DataSourceConfiguration save(DataSourceConfiguration configuration) {
        return dataSourceRepository.save(configuration);
    }

    public void remove(DataSourceConfiguration configuration) {
        dataSourceRepository.delete(configuration);
    }

    private ProductRepository getProductRepository() {
        if (productRepository == null) {
            productRepository = new ProductRepository(this);
        }
        return productRepository;
    }

    private DownloadProductRepository getDownloadProductRepository() {
        if (downloadProductRepository == null) {
            downloadProductRepository = new DownloadProductRepository(this);
        }
        return downloadProductRepository;
    }

    private DownloadProductTileRepository getDownloadProductTileRepository() {
        if (downloadProductTileRepository == null) {
            downloadProductTileRepository = new DownloadProductTileRepository(this);
        }
        return downloadProductTileRepository;
    }

    private NonMappedEntitiesRepository getNonMappedEntitiesRepository() {
        if (nonMappedEntitiesRepository == null) {
            nonMappedEntitiesRepository = new NonMappedEntitiesRepository(this);
        }
        return nonMappedEntitiesRepository;
    }

    @Transactional(readOnly = true)
    public List<DownloadProduct> getProducts(int startPage, int pageSize, boolean dateDescending) {
        List<DownloadProduct> products = null;
        Sort.Direction sortDirection = dateDescending ? Sort.Direction.DESC : Sort.Direction.ASC;
        // retrieve products
        if (startPage > 0 && pageSize > 0) {
            PageRequest page = PageRequest.of(startPage, pageSize, sortDirection, "productDate");
            products = getDownloadProductRepository().findAll(page);
        } else {
            products = getDownloadProductRepository().findAll(new Sort(sortDirection, "productDate"));
        }
        return products;
    }

    public DataSource getDataSource() { return dbConfig.dataSource(); }

    public DataSourceConfiguration getDataSourceConfiguration(int id) {
        return dataSourceRepository.findById(id).orElse(null);
    }

    public List<Site> getEnabledSites() {
        return siteRepository.getEnabledSites();
    }

    public Set<String> getSiteTiles(Site site, Satellite satellite) {
        final SiteTiles tiles = getNonMappedEntitiesRepository().getSiteTiles(site.getId(), satellite.value());
        Set<String> tileList = null;
        if (tiles != null) {
            String[] tileNames = tiles.getTiles();
            if (tileNames != null && tileNames.length > 0) {
                tileList = new HashSet<>();
                tileList.addAll(Arrays.asList(tileNames));
            }
        }
        return tileList;
    }

    @Transactional
    public List<DataSourceConfiguration> getDataSourceConfigurations() {
        List<DataSourceConfiguration> configurations = new ArrayList<>();
        final List<DataSourceConfiguration> dbConfigs = dataSourceRepository.getConfigurations();
        final List<DataSourceConfiguration> availableConfigs = registeredDataSources();
        if (dbConfigs == null || dbConfigs.size() == 0) {
            configurations.addAll(availableConfigs);
            dataSourceRepository.saveAll(configurations);
        } else {
            final List<DataSourceConfiguration> toRemove = dbConfigs.stream()
                    .filter(not(availableConfigs::contains))
                    .collect(Collectors.toList());
            final List<DataSourceConfiguration> toAdd = availableConfigs.stream()
                    .filter(not(dbConfigs::contains))
                    .collect(Collectors.toList());
            dbConfigs.removeAll(toRemove);
            dataSourceRepository.deleteAll(toRemove);
            dataSourceRepository.saveAll(toAdd);
            configurations.addAll(dbConfigs);
            configurations.addAll(toAdd);
        }
        return configurations;//.stream().filter(DataSourceConfiguration::isEnabled).collect(Collectors.toList());
    }

    public LocalDateTime getLastDownloadDate(short siteId, short satelliteId) {
        final DownloadProduct lastProduct = getDownloadProductRepository().findLastProduct(siteId, satelliteId);
        return lastProduct != null ? lastProduct.getProductDate() : null;
    }

    public List<DownloadProduct> getRetriableProducts(short siteId, DataSourceConfiguration configuration) {
        return getDownloadProductRepository().findRetriesBySite(siteId, configuration.getSatellite().value());
    }

    public List<DownloadProduct> getLastRetriableProducts(short siteId, DataSourceConfiguration configuration) {
        return getDownloadProductRepository().findLastRetriesBySite(siteId, configuration.getSatellite().value());
    }

    public List<DownloadProduct> getProductsWithoutOrbitDirection(int siteId, int satelliteId) {
        return getDownloadProductRepository().findWithoutOrbits(siteId, satelliteId);
    }

    public List<HighLevelProduct> getMovableProducts(Site site, Satellite satellite) {
        return getProductRepository().findMovableProducts(site.getId(), satellite.value());
    }

    public List<HighLevelProduct> getMovableProducts(Site site, Set<Integer> productTypes) {
        return getProductRepository().findMovableProducts(site.getId(), productTypes);
    }

    public HighLevelProduct setArchived(HighLevelProduct product) {
        return getProductRepository().setArchived(product);
    }

    public DownloadProduct getProductByName(int siteId, String productName) {
        return getDownloadProductRepository().findByName(siteId, productName);
    }

    public List<String> getOtherSitesProducts(int currentSiteId, Set<String> productNames) {
        List<DownloadProduct> results = getDownloadProductRepository().getOtherSitesExistingProducts(currentSiteId, productNames);
        return results != null ? results.stream().map(DownloadProduct::getProductName).collect(Collectors.toList()) : null;
    }

    public Set<String> findExistingProducts(List<String> productNames) {
        return new HashSet<>(getDownloadProductRepository().findExistingProducts(productNames));
    }

    public void attachToSite(Site site, List<String> productNames) {
        if (site == null || site.getId() == 0 || productNames == null || productNames.size() == 0) {
            return;
        }
        DownloadProductRepository repository = getDownloadProductRepository();
        DownloadProductTileRepository tileRepository = getDownloadProductTileRepository();
        List<DownloadProduct> products = repository.getExistingProducts(productNames);
        for (DownloadProduct product : products) {
            if (product.getSiteId() != site.getId()) {
                DownloadProduct duplicate = product.duplicate();
                duplicate.setSiteId(site.getId());
                duplicate.setTimestamp(LocalDateTime.now());
                Set<String> siteTiles = getSiteTiles(site, product.getSatelliteId());
                String[] productTiles = duplicate.getTiles();
                if (siteTiles != null && productTiles != null && productTiles.length > 0) {
                    if (Arrays.stream(productTiles).anyMatch(siteTiles::contains)) {
                        duplicate.setStatusId(Status.DOWNLOADED);
                    } else {
                        duplicate.setStatusId(Status.IGNORED);
                        duplicate.setStatusReason(String.format("Product doesn't contain any tile defined for the site '%s'",
                                                                site.getShortName()));
                    }
                } else {
                    duplicate.setStatusId(Status.DOWNLOADED);
                }
                duplicate.setStatusReason(null);
                duplicate.setNbRetries((short) 0);
                duplicate = repository.save(duplicate);
                List<DownloadProductTile> tiles = tileRepository.findByProductId(product.getId());
                if (tiles != null) {
                    for (DownloadProductTile tile : tiles) {
                        DownloadProductTile duplicateTile = tile.duplicate();
                        duplicateTile.setDownloadProductId(duplicate.getId());
                        tileRepository.save(duplicateTile);
                    }
                }
            }
        }
    }

    public List<DownloadProduct> getProducts(int siteId, int satelliteId, Status... statuses) {
        DownloadProductRepository repository = getDownloadProductRepository();
        return repository.findByStatus(siteId, satelliteId, statuses);
    }

    public List<DownloadProduct> getDownloadedProducts(int siteId, Satellite satellite) {
        DownloadProductRepository repository = getDownloadProductRepository();
        return repository.findByStatus(siteId, satellite.value(),
                                       Status.DOWNLOADED, Status.PROCESSING, Status.PROCESSING_FAILED, Status.PROCESSED);
    }

    public List<DownloadProduct> getDownloadedProducts(int siteId, Satellite satellite, LocalDate olderThan) {
        return getDownloadProductRepository().findByStatusAndDate(siteId, satellite.value(), olderThan);
    }

    public List<DownloadProduct> getStalledProducts(int siteId, int daysBack) {
        return getDownloadProductRepository().findPreviouslyNotIntersected(siteId, daysBack);
    }

    public List<DownloadProduct> getIntersectingProducts(String productName, int daysBack, double threshold) {
        return getDownloadProductRepository().findIntersectingProducts(productName, daysBack, threshold);
    }

    public List<DownloadProductTile> getDownloadProductTiles(int siteId) {
        return getDownloadProductTileRepository().findBySite(siteId);
    }

    public List<DownloadProductTile> getDownloadProductTiles(int siteId, Satellite satellite) {
        return getDownloadProductTileRepository().findBySiteAndSatellite(siteId, satellite);
    }

    public List<DownloadProductTile> getDownloadProductTilesByProduct(int productId) {
        return getDownloadProductTileRepository().findByProductId(productId);
    }

    public List<DownloadProductTile> getDownloadProductTiles(PageRequest pageRequest) {
        return getDownloadProductTileRepository().findAll(pageRequest);
    }

    public List<DownloadProductTile> getDownloadProductTiles(TileProcessingStatus... statuses) {
        return getDownloadProductTileRepository().findByStatus(statuses);
    }

    public List<DownloadProductTile> getDownloadProductTiles(int siteId, TileProcessingStatus... statuses) {
        return getDownloadProductTileRepository().findBySiteAndStatus(siteId, statuses);
    }

    public DownloadProductTile save(DownloadProductTile tile) {
        return getDownloadProductTileRepository().save(tile);
    }

    public int deleteProducts(List<DownloadProduct> products) {
        if (products == null || products.size() == 0) {
            return 0;
        }
        int deleted = 0;
        Set<Short> siteIds = products.stream().map(DownloadProduct::getSiteId).distinct().collect(Collectors.toSet());
        DownloadProductRepository repository = getDownloadProductRepository();
        for (short siteId : siteIds) {
            deleted += repository.delete(siteId,
                                         products.stream().filter(p -> p.getSiteId() == siteId)
                                                 .map(DownloadProduct::getId).toArray(Integer[]::new));
        }
        return deleted;
    }

    public int countCompleted(int siteId, Satellite satellite, LocalDate start, LocalDate end) {
        Integer completed = getDownloadProductRepository().countByStatus(siteId, satellite.value(),
                                                                         Status.DOWNLOADED.value(), start, end);
        Integer failed = getDownloadProductRepository().countByStatus(siteId, satellite.value(),
                                                                      Status.ABORTED.value(), start, end);
        return (completed == null ? 0 : completed) + (failed == null ? 0 : failed);
    }

    public List<ProductCount> countEstimated(int siteId, Satellite satellite) {
        return new NonMappedEntitiesRepository(this).getEstimatedProductCount(siteId, satellite.value());

    }

    public List<DownloadSummary> countProducts(int siteId) {
        return getDownloadProductRepository().downloadHistoryReport(siteId);
    }

    @Transactional
    public ProductCount save(ProductCount productCount) {
        try {
            new NonMappedEntitiesRepository(this).save(productCount);
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return productCount;
    }

    public List<HighLevelProduct> getProducedProducts(int siteId, ProductType... productTypes) {
        if (productTypes != null && productTypes.length > 0) {
            return getProductRepository().findProducts(siteId,Arrays.stream(productTypes)
                                                        .map(ProductType::value)
                                                        .collect(Collectors.toSet()));
        } else {
            return new ArrayList<>();
        }
    }

    public List<HighLevelProduct> getProducedProducts(int downloadHistoryId) {
        return getProductRepository().findByDownloadedProduct(downloadHistoryId);
    }

    public HighLevelProduct getHighLevelProductByName(int siteId, String name) {
        return getProductRepository().findProductByName(siteId, name);
    }

    public HighLevelProduct save(HighLevelProduct product) {
        return product != null ?  getProductRepository().saveProduct(product) : null;
    }

    public ProductDetails getProductStatistics(int productId) {
        return getNonMappedEntitiesRepository().getProductStatistics(productId);
    }

    public List<ProductDetails> getStatisticsForProducts(Set<Integer> productIds) {
        return getNonMappedEntitiesRepository().getStatisticsForProducts(productIds);
    }

    public void save(ProductDetails productDetails) {
        try {
            getNonMappedEntitiesRepository().save(productDetails);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public Processor getProcessor(String shortName) {
        return processorRepository.findByShortName(shortName);
    }

    public List<Season> getEnabledSeasons(Short siteId) {
        if (siteId == null) {
            return seasonRepository.getEnabledSeasons();
        } else {
            return seasonRepository.getEnabledSeasons(siteId);
        }
    }

    public List<Season> getSeasons(Short siteId) {
        if (siteId == null) {
            return seasonRepository.getSeasons();
        } else {
            return seasonRepository.getSeasons(siteId);
        }
    }

    public List<ProductTypeInfo> getProductTypes() {
        return productTypeRepository.getProductTypes();
    }

    public List<ConfigurationItem> getConfigurationItem(String key) {
        return configRepository.getByKeyStartingWith(key);
    }

    public Map<Short, Map<String, String>> getConfiguration() {
        Map<Short, Map<String, String>> propertiesMap = new HashMap<>();
        // get enabled sites
        final List<Site> sitesInfo = siteRepository.getAllSites();
        final ArrayList<ConfigurationItem> list = StreamSupport.stream(configRepository.findAll().spliterator(), false).collect(Collectors.toCollection(ArrayList::new));
        Map<String, String> common = list.stream().filter(c -> c.getSiteId() == null || c.getSiteId() == 0)
                .collect(Collectors.toMap(ConfigurationItem::getKey, ConfigurationItem::getValue));
        propertiesMap.put((short) 0, common);
        sitesInfo.forEach(site -> {
            final short siteId = site.getId();
            propertiesMap.put(siteId, new HashMap<>());
            final Map<String, String> siteSettings = propertiesMap.get(siteId);
            siteSettings.putAll(common);
            siteSettings.putAll(list.stream().filter(c -> c.getSiteId() != null && siteId == c.getSiteId())
                    .collect(Collectors.toMap(ConfigurationItem::getKey, ConfigurationItem::getValue)));
        });
        return propertiesMap;
    }

    @Transactional
    public void saveSetting(short siteId, String name, String value) {
        List<ConfigurationItem> configurationItems = configRepository.getByKeyStartingWith(name);
        ConfigurationItem item = (configurationItems == null || configurationItems.size() == 0) ?
                new ConfigurationItem() :
                configurationItems.stream()
                                .filter(ci -> (siteId == 0 && ci.getSiteId() == null) || siteId == (ci.getSiteId() == null ? 0 : ci.getSiteId()))
                                .findFirst().orElseGet(ConfigurationItem::new);
        item.setKey(name);
        item.setSiteId(siteId != 0 ? siteId : null);
        item.setValue(value);
        configRepository.save(item);
    }

    @Transactional
    public String getSetting(short siteId, String name, String defaultVal) {
        List<ConfigurationItem> configurationItems = configRepository.getByKeyStartingWith(name);
        if (configurationItems == null || configurationItems.size() == 0) {
            return defaultVal;
        } else {
            ConfigurationItem item = configurationItems.stream()
                    .filter(ci -> (siteId == 0 && ci.getSiteId() == null) || (siteId == (ci.getSiteId() == null ? 0 : ci.getSiteId())))
                    .findFirst().orElse(null);
            return item != null ? item.getValue() : defaultVal;
        }
    }

    @Transactional
    public void deleteSite(short siteId) {
        final DataSource dataSource = dbConfig.dataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        String[] statements = new String[] {
                String.format("delete from site_tiles where (site_id = %s)", siteId),
                String.format("delete from config where (site_id = %s)", siteId),
                String.format("delete from step where task_id in (select id from task where job_id in (select id from job where site_id = %s))",
                              siteId),
                String.format("delete from step_resource_log where task_id in (select id from task where job_id in (select id from job where site_id = %s))",
                              siteId),
                String.format("delete from task where job_id in (select id from job where site_id = %s)", siteId),
                String.format("delete from config_job where job_id in (select id from job where site_id = %s)", siteId),
                String.format("delete from job where (site_id = %s)", siteId),
                String.format("delete from scheduled_task_status where task_id in (select id from scheduled_task where site_id = %s)", siteId),
                String.format("delete from scheduled_task where (site_id = %s)", siteId),
                String.format("delete from l1_tile_history where downloader_history_id in (select id from downloader_history where site_id = %s)", siteId),
                String.format("delete from downloader_history where (site_id = %s)", siteId),
                String.format("delete from downloader_count where (site_id = %s)", siteId),
                String.format("delete from product where (site_id = %s)", siteId),
                String.format("delete from season where (site_id = %s)", siteId),
                String.format("delete from config where (site_id = %s)", siteId),
                String.format("delete from site where (id = %s)", siteId)
        };
        jdbcTemplate.batchUpdate(statements);
    }

    @Transactional(readOnly = true)
    public List<Site> getAllSites() {
        // get all sites
        return siteRepository.getAllSites();
    }

    @Transactional(readOnly = true)
    public Site getSiteByShortName(String shortName) {
        return siteRepository.getSiteByShortName(shortName);
    }

    @Transactional(readOnly = true)
    public Site getSiteById(Short id) {
        return siteRepository.getSiteById(id);
    }

    private List<DataSourceConfiguration> registeredDataSources() {
        List<DataSourceConfiguration> configurations = new ArrayList<>();
        final DataSourceManager dataSourceManager = DataSourceManager.getInstance();
        final SortedSet<String> sensors = dataSourceManager.getSupportedSensors();
        for (String sensor : sensors) {
            final Satellite satellite = Enum.valueOf(Satellite.class, sensor);
            final List<String> dataSourceNames = dataSourceManager.getNames(sensor);
            dataSourceNames.sort(Comparator.reverseOrder());
            int scopeMask = dataSourceNames.size() > 1 ? Scope.QUERY : Scope.QUERY | Scope.DOWNLOAD;
            for (String dataSourceName : dataSourceNames) {
                String downloadPath = Config.getSetting(String.format(ConfigurationKeys.DOWNLOAD_DIR,
                                                                      satellite.shortName().toLowerCase()),
                                                        Constants.DEFAULT_TARGET_PATH);
                final ro.cs.tao.datasource.DataSource dataSource = dataSourceManager.get(sensor, dataSourceName);
                DataSourceConfiguration configuration = new DataSourceConfiguration();
                configuration.setDataSourceName(dataSourceName);
                configuration.setSatellite(satellite);
                configuration.setFetchMode(Constants.DEFAULT_FETCH_MODE);
                configuration.setScope(scopeMask);
                configuration.setMaxRetries(Constants.DEFAULT_MAX_RETRIES);
                configuration.setMaxConnections(Constants.DEFAULT_MAX_CONNECTIONS);
                configuration.setRetryInterval(Constants.DEFAULT_RETRY_INTERVAL);
                configuration.setDownloadPath(downloadPath);
                Map<String, Map<String, DataSourceParameter>> allSourceParams =
                        dataSource.getSupportedParameters();
                List<DataSourceParameter> params = allSourceParams.get(sensor).values()
                        .stream().filter(p -> p.getDefaultValue() != null).collect(Collectors.toList());
                if (params != null) {
                    for (DataSourceParameter param : params) {
                        configuration.setParameter(param.getName(), param.getType(), param.getDefaultValue());
                    }
                }
                configuration.setEnabled(true);
                configurations.add(configuration);
                if (((scopeMask <<= 1) & Scope.DOWNLOAD) == 0) {
                    break;
                }
            }
        }
        return configurations;
    }

    private static <T> Predicate<T> not(Predicate<T> predicate) {
        return predicate.negate();
    }
}
