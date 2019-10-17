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

import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.DownloadSummary;
import org.esa.sen2agri.entities.converters.OrbitTypeConverter;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.entities.converters.StatusConverter;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.entities.enums.Status;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Sort;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.support.GeneratedKeyHolder;
import org.springframework.jdbc.support.KeyHolder;
import ro.cs.tao.serialization.GeometryAdapter;

import javax.sql.DataSource;
import java.sql.Date;
import java.sql.*;
import java.time.LocalDate;
import java.util.*;
import java.util.stream.Collectors;

public class DownloadProductRepository extends NonMappedRepository<DownloadProduct> {

    DownloadProductRepository(PersistenceManager persistenceManager) {
        super(persistenceManager);
    }

    List<DownloadProduct> findRetriesBySite(int siteId, int satelliteId) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "JOIN datasource d ON d.satellite_id = dh.satellite_id " +
                        "WHERE dh.site_id = ? AND dh.status_id IN (1, 3) AND dh.satellite_id = ? " +
                        "AND dh.no_of_retries < d.max_retries " +
                        "AND EXTRACT(epoch FROM (current_timestamp - dh.created_timestamp)) / 60 >= d.retry_interval_minutes " +
                        "AND d.scope IN (2, 3) AND d.enabled = true ORDER BY dh.product_date";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.list();
    }

    List<DownloadProduct> findLastRetriesBySite(int siteId, int satelliteId) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "JOIN datasource d ON d.satellite_id = dh.satellite_id " +
                        "WHERE dh.site_id = ? AND dh.status_id IN (1, 3) AND dh.satellite_id = ? " +
                        "AND dh.no_of_retries = d.max_retries " +
                        "AND EXTRACT(epoch FROM (current_timestamp - dh.created_timestamp)) / 60 >= d.retry_interval_minutes " +
                        "AND d.scope IN (2, 3) AND d.enabled = true ORDER BY dh.product_date";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.list();
    }

    List<DownloadProduct> findWithoutOrbits(int siteId, int satelliteId) {
        if (Satellite.Sentinel1.value() != satelliteId) {
            return new ArrayList<>();
        }
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE dh.site_id = ? AND dh.satellite_id = ? " +
                        "AND dh.orbit_type_id IS NULL ORDER BY dh.product_date";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.list();
    }

    DownloadProduct findDownloadProduct(int productId) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() { return "WHERE dh.id = ?"; }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, productId);
            }
        }.single();
    }

    List<DownloadProduct> findAll(PageRequest pageRequest) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("ORDER BY %s OFFSET %s LIMIT %s,",
                                     String.join(",", pageRequest.getSort().stream()
                                             .map(o -> o.getProperty() + " " + o.getDirection().name())
                                             .collect(Collectors.toList())),
                                     pageRequest.getOffset(),
                                     pageRequest.getPageSize());
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    List<DownloadProduct> findAll(Sort sort) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("ORDER BY %s",
                                     sort.stream()
                                             .map(o -> o.getProperty() + " " + o.getDirection().name())
                                             .collect(Collectors.toList()));
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    DownloadProduct findLastProduct(int siteId, int satelliteId) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE dh.site_id = ? AND dh.status_id IN (2, 4, 41, 5) AND dh.satellite_id = ? " +
                        "ORDER BY dh.product_date DESC LIMIT 1";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.single();
    }

    DownloadProduct findByName(int siteId, String productName) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE dh.site_id = ? AND dh.product_name LIKE CONCAT(CAST(? AS varchar), '%')";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setString(2, productName);
            }
        }.single();
    }

    List<String> findExistingProducts(List<String> productNames) {
        List<DownloadProduct> results = getExistingProducts(productNames);
        return results != null ? results.stream().map(DownloadProduct::getProductName).collect(Collectors.toList()) : null;
    }

    List<DownloadProduct> getExistingProducts(List<String> productNames) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("WHERE dh.product_name IN (%s) and dh.status_id IN (2, 5, 6, 7)",
                                     String.join(",", productNames.stream().map(n -> "'" + n + "'").collect(Collectors.toSet())));
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    List<DownloadProduct> getOtherSitesExistingProducts(int siteId, Set<String> productNames) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("WHERE dh.site_id != ? AND dh.product_name IN (%s) AND dh.status_id IN (2, 41, 5, 6, 7)",
                                     String.join(",", productNames.stream().map(n -> "'" + n + "'").collect(Collectors.toSet())));
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
            }
        }.list();
    }

    List<DownloadProduct> findByStatus(int siteId, int satelliteId, Status... statuses) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                if (statuses == null || statuses.length == 0) {
                    return "WHERE dh.site_id = ? AND dh.satellite_id = ? ORDER BY dh.product_date";
                } else {
                    return String.format("WHERE dh.site_id = ? AND dh.satellite_id = ? AND dh.status_id IN (%s) ORDER BY dh.product_date",
                                         String.join(",", Arrays.stream(statuses)
                                                                .map(s -> String.valueOf(s.value()))
                                                                .collect(Collectors.toList())));
                }
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.list();
    }

    List<DownloadProduct> findByStatusAndDate(int siteId, int satelliteId, LocalDate date, boolean latestFirst) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE dh.site_id = ? AND dh.satellite_id = ? AND ((dh.status_id = ? AND dh.product_date <= ?) OR (dh.status_id = ? AND dh.no_of_retries < 3)) " +
                        "ORDER BY dh.product_date " + (latestFirst ? "DESC" : "ASC");
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
                statement.setInt(2, satelliteId);
                statement.setInt(3, Status.DOWNLOADED.value());
                statement.setDate(4, Date.valueOf(date));
                statement.setInt(5, Status.PROCESSING_FAILED.value());
            }
        }.list();
    }

    List<DownloadProduct> findPreviouslyNotIntersected(int siteId, int daysBack) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "JOIN (SELECT d2.product_name, i.product_name AS intr, d2.site_id, d2.orbit_id, d2.satellite_id FROM downloader_history d2 " +
                        "JOIN downloader_history i ON i.site_id = d2.site_id AND i.orbit_id = d2.orbit_id AND i.satellite_id = d2.satellite_id " +
                        "WHERE ST_INTERSECTS(i.footprint, d2.footprint) AND DATE_PART('day', d2.product_date - i.product_date) BETWEEN ? AND ? AND d2.satellite_id = 3) AS intersections " +
                        "ON intersections.site_id = dh.site_id AND intersections.orbit_id = dh.orbit_id AND intersections.satellite_id = dh.satellite_id AND " +
                        "intersections.product_name = dh.product_name " +
                        "LEFT JOIN (SELECT full_path, name, site_id, orbit_id, satellite_id, inserted_timestamp, CASE WHEN array_length(string_to_array(name, '_'), 1) = 8 THEN " +
                        "GREATEST(substr(split_part(name, '_', 4), 2, 15)::date, substr(split_part(name, '_', 5), 1, 15)::date) ELSE " +
                        "GREATEST(substr(split_part(name, '_', 6), 2, 15)::date, substr(split_part(name, '_', 7), 1, 15)::date) END as master_date FROM product) AS p " +
                        "ON p.site_id = dh.site_id AND p.orbit_id = dh.orbit_id AND p.satellite_id = dh.satellite_id AND CAST(dh.product_date as date) = p.master_date " +
                        "WHERE dh.satellite_id = 3 AND dh.site_id = ? AND dh.status_id = 5 AND p.full_path IS NULL " +
                        "ORDER BY dh.product_date, dh.orbit_id, dh.product_name;";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, daysBack - 1);
                statement.setInt(2, daysBack + 1);
                statement.setInt(3, siteId);
            }
        }.list();
    }

    List<DownloadProduct> findIntersectingProducts(String productName, int daysBack, double threshold) {
        return new DownloadProductTemplate() {
            @Override
            protected String baseSQL() {
                return "SELECT dh.id, dh.site_id, dh.satellite_id, dh.product_name, " +
                        "dh.full_path, dh.created_timestamp, dh.status_id, dh.no_of_retries, " +
                        "dh.product_date, dh.orbit_id, dh.status_reason, dh.tiles, dh.footprint, dh.orbit_type_id " +
                        "FROM downloader_history i " +
                        "JOIN downloader_history dh on dh.site_id = i.site_id AND dh.satellite_id = i.satellite_id AND dh.orbit_id = i.orbit_id ";
            }
            @Override
            protected String conditionsSQL() {
                return "WHERE i.product_name = ? " +
                        "AND ST_INTERSECTS(dh.footprint, i.footprint) " +
                        "AND DATE_PART('day', i.product_date - dh.product_date) BETWEEN ? AND ? " +
                        "AND st_area(st_intersection(dh.footprint, i.footprint)) / st_area(dh.footprint) > ? " +
                        "ORDER BY st_area(st_intersection(dh.footprint, i.footprint)) / st_area(dh.footprint) DESC, i.product_date ASC";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setString(1, productName);
                statement.setInt(2, daysBack - 1);
                statement.setInt(3, daysBack + 1);
                statement.setDouble(4, threshold);
            }
        }.list();
    }

    Integer countByStatus(int siteId, int satelliteId, int statusId, LocalDate start, LocalDate end) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        return jdbcTemplate.query(
                connection -> {
                    PreparedStatement statement =
                            connection.prepareStatement("SELECT COUNT(*) FROM downloader_history " +
                                                                "WHERE site_id = ? AND status_id = ? AND satellite_id = ? " +
                                                                "AND product_date BETWEEN ? AND ?");
                    statement.setShort(1, (short) siteId);
                    statement.setShort(2, (short) statusId);
                    statement.setDate(3, Date.valueOf(start));
                    statement.setDate(4, Date.valueOf(end));
                    return statement;
                },
                (resultSet, i) -> resultSet.getInt(1)).stream().findFirst().orElse(0);
    }

    List<DownloadSummary> downloadHistoryReport(int siteId) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        final SatelliteConverter satelliteConverter = new SatelliteConverter();
        final StatusConverter statusConverter = new StatusConverter();
        return jdbcTemplate.query(
                connection -> {
                    PreparedStatement statement =
                            connection.prepareStatement("SELECT s.short_name, date_part('year', d.product_date) as \"year\", d.satellite_id, d.status_id, COUNT(d.id) " +
                                                                "FROM downloader_history d " +
                                                                "JOIN site s ON s.id = d.site_id " +
                                                                "JOIN downloader_status st ON st.id = d.status_id " +
                                                                "WHERE s.enabled = true AND s.id = ? AND d.satellite_id = 3 " +
                                                                "GROUP BY s.short_name, \"year\", d.satellite_id, d.status_id " +
                                                                "ORDER BY s.short_name, \"year\", d.satellite_id, d.status_id;");
                    statement.setInt(1, siteId);
                    return statement;
                },
                (resultSet, i) -> {
                    DownloadSummary row = new DownloadSummary();
                    row.setSiteName(resultSet.getString(1));
                    row.setYear(resultSet.getInt(2));
                    row.setSatellite(satelliteConverter.convertToEntityAttribute(resultSet.getShort(3)));
                    row.setStatus(statusConverter.convertToEntityAttribute(resultSet.getShort(4)));
                    row.setCount(resultSet.getInt(5));
                    return row;
                });
    }

    DownloadProduct save(DownloadProduct product) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        KeyHolder keyHolder = new GeneratedKeyHolder();
        final boolean isUpdateQuery = findByName(product.getSiteId(), product.getProductName()) != null;
        Connection conn;
        jdbcTemplate.update(connection -> {
            PreparedStatement statement = connection.prepareStatement(isUpdateQuery ? updateQuery() : insertQuery(), Statement.RETURN_GENERATED_KEYS);
            statement.setShort(1, product.getSiteId());
            statement.setShort(2, product.getSatelliteId().value());
            statement.setString(3, product.getProductName());
            statement.setString(4, product.getFullPath());
            statement.setTimestamp(5, Timestamp.valueOf(product.getTimestamp()));
            statement.setInt(6, product.getStatusId().value());
            statement.setShort(7, product.getNbRetries());
            statement.setTimestamp(8, Timestamp.valueOf(product.getProductDate()));
            statement.setInt(9, product.getOrbitId());
            statement.setString(10, product.getStatusReason());
            String[] tiles = product.getTiles();
            Array array = tiles != null ? connection.createArrayOf("text", tiles) : null;
            statement.setArray(11, array);
            statement.setString(12, new GeometryAdapter().unmarshal(product.getFootprint()));
            if (product.getOrbitType() != null) {
                statement.setInt(13, product.getOrbitType().value());
            } else {
                statement.setNull(13, Types.SMALLINT);
            }
            if (isUpdateQuery) {
                statement.setShort(14, product.getSiteId());
                statement.setString(15, product.getProductName());
            }
            return statement;
        }, keyHolder);
        try {
            Map<String, Object> keys = keyHolder.getKeys();
            Integer id = null;
            if (keys != null) {
                id = (Integer) keys.get("id");
            }
            return id != null ? findDownloadProduct(id) : product;
        } catch (Exception e) {
            return product;
        }
    }

    int delete(int siteId, Integer... productIds) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                if (productIds != null && productIds.length > 0) {
                    return String.format("WHERE site_id = ? AND id IN (%s)",
                                         String.join(",", Arrays.stream(productIds).map(String::valueOf)
                                                                .collect(Collectors.toList())));
                } else {
                    return "WHERE 1 != 2"; // do not allow deleting all products
                }
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
            }
        }.delete();
    }

    int delete(int siteId, String... productNames) {
        return new DownloadProductTemplate() {
            @Override
            protected String conditionsSQL() {
                if (productNames != null && productNames.length > 0) {
                    return String.format("WHERE site_id = ? AND product_name IN (%s)",
                                         String.join(",", Arrays.stream(productNames).collect(Collectors.toList())));
                } else {
                    return "WHERE 1 != 2"; // do not allow deleting all products
                }
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
            }
        }.delete();
    }

    @Override
    protected String selectQuery() {
        return "SELECT dh.id, dh.site_id, dh.satellite_id, dh.product_name, " +
                "dh.full_path, dh.created_timestamp, dh.status_id, dh.no_of_retries, " +
                "dh.product_date, dh.orbit_id, dh.status_reason, dh.tiles, dh.footprint, dh.orbit_type_id " +
                "FROM downloader_history dh ";
    }

    @Override
    protected String insertQuery() {
        return "INSERT INTO downloader_history(site_id, satellite_id, product_name, " +
                "full_path, created_timestamp, status_id, no_of_retries, product_date, " +
                "orbit_id, status_reason, tiles, footprint, orbit_type_id) " +
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }

    @Override
    protected String updateQuery() {
        return "UPDATE downloader_history " +
                "SET site_id=?, satellite_id=?, product_name=?, full_path=?, created_timestamp=?, status_id=?, " +
                "no_of_retries=?, product_date=?, orbit_id=?, status_reason=?, tiles=?, footprint=?, orbit_type_id=? " +
                "WHERE site_id=? and product_name=?";
    }

    @Override
    protected String deleteQuery() {
        return "DELETE FROM downloader_history";
    }

    private abstract class DownloadProductTemplate extends Template {
        @Override
        protected String baseSQL() { return selectQuery(); }

        @Override
        protected RowMapper<DownloadProduct> rowMapper() {
            return (resultSet, rowNum) -> {
                DownloadProduct product = new DownloadProduct();
                product.setId(resultSet.getInt(1));
                product.setSiteId(resultSet.getShort(2));
                product.setSatelliteId(new SatelliteConverter().convertToEntityAttribute(resultSet.getShort(3)));
                product.setProductName(resultSet.getString(4));
                product.setFullPath(resultSet.getString(5));
                Timestamp timestamp = resultSet.getTimestamp(6);
                if (timestamp != null) {
                    product.setTimestamp(timestamp.toLocalDateTime());
                }
                product.setStatusId(new StatusConverter().convertToEntityAttribute(resultSet.getShort(7)));
                product.setNbRetries(resultSet.getShort(8));
                timestamp = resultSet.getTimestamp(9);
                if (timestamp != null) {
                    product.setProductDate(timestamp.toLocalDateTime());
                }
                product.setOrbitId(resultSet.getInt(10));
                product.setStatusReason(resultSet.getString(11));
                try {
                    Array array = resultSet.getArray(12);
                    if (array != null) {
                        product.setTiles((String[]) array.getArray());
                    }
                } catch (Exception ignored) { }
                try {
                    product.setFootprint(new GeometryAdapter().marshal(resultSet.getString(13)));
                } catch (Exception e) {
                    logger.warning("Cannot recreate footprint from database. Reason: " + e.getMessage());
                }
                product.setOrbitType(new OrbitTypeConverter().convertToEntityAttribute(resultSet.getInt(14)));
                return product;
            };
        }
    }

}
