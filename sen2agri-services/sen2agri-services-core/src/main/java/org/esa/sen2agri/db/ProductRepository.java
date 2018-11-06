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

import org.esa.sen2agri.entities.HighLevelProduct;
import org.esa.sen2agri.entities.converters.OrbitTypeConverter;
import org.esa.sen2agri.entities.converters.ProductTypeConverter;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.core.SqlParameter;
import org.springframework.jdbc.core.simple.SimpleJdbcCall;
import ro.cs.tao.serialization.GeometryAdapter;
import ro.cs.tao.utils.StringUtilities;

import javax.sql.DataSource;
import java.sql.*;
import java.time.LocalDateTime;
import java.util.*;
import java.util.stream.Collectors;

public class ProductRepository extends NonMappedRepository<HighLevelProduct> {
    private static final SqlParameter[] PRODUCT_INSERT_PARAMS = new SqlParameter[] {
            new SqlParameter("_product_type_id", Types.SMALLINT),
            new SqlParameter("_processor_id", Types.SMALLINT),
            new SqlParameter("_satellite_id", Types.INTEGER),
            new SqlParameter("_site_id", Types.SMALLINT),
            new SqlParameter("_job_id", Types.INTEGER),
            new SqlParameter("_full_path", Types.VARCHAR),
            new SqlParameter("_created_timestamp", Types.TIMESTAMP),
            new SqlParameter("_name", Types.VARCHAR),
            new SqlParameter("_quicklook_image", Types.VARCHAR),
            new SqlParameter("_footprint", Types.OTHER),
            new SqlParameter("_orbit_id", Types.INTEGER),
            new SqlParameter("_tiles", Types.VARCHAR),
            new SqlParameter("_orbit_type_id", Types.SMALLINT),
            new SqlParameter("_downloader_history_id", Types.INTEGER)
    };

    ProductRepository(PersistenceManager persistenceManager) {
        super(persistenceManager);
    }

    List<HighLevelProduct> findMovableProducts(int siteId, int satelliteId) {
        return new HighLevelProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE site_id = ? AND satellite_id = ? AND is_archived = false ORDER BY inserted_timestamp";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
                statement.setShort(2, (short) satelliteId);
            }
        }.list();
    }

    List<HighLevelProduct> findMovableProducts(int siteId, Set<Integer> productTypeIds) {
        return new HighLevelProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE site_id = ? AND product_type_id in (" +
                        String.join(",", productTypeIds.stream().map(Object::toString).collect(Collectors.toList())) + ") " +
                        "AND is_archived = false ORDER BY inserted_timestamp";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
            }
        }.list();
    }

    List<HighLevelProduct> findProducts(int siteId, Set<Integer> productTypeIds) {
        return new HighLevelProductTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE site_id = ? AND product_type_id in (" +
                        String.join(",", productTypeIds.stream().map(Object::toString).collect(Collectors.toList())) + ")";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setShort(1, (short) siteId);
            }
        }.list();
    }

    HighLevelProduct findProduct(int productId) {
        return new HighLevelProductTemplate() {
            @Override
            protected String conditionsSQL() { return "WHERE id = ?"; }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, productId);
            }
        }.single();
    }

    List<HighLevelProduct> findByDownloadedProduct(int downloadHistoryId) {
        return new HighLevelProductTemplate() {
            @Override
            protected String conditionsSQL() { return "WHERE downloader_history_id = ?"; }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, downloadHistoryId);
            }
        }.list();
    }

    HighLevelProduct saveProduct(HighLevelProduct product) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        SimpleJdbcCall insertProductCall = new SimpleJdbcCall(jdbcTemplate)
                .withFunctionName("sp_insert_product")
                .declareParameters(PRODUCT_INSERT_PARAMS)
                .withReturnValue()
                .withoutProcedureColumnMetaDataAccess();
        Map<String, Object> params = new HashMap<>();
        params.put("_product_type_id", product.getProductType().value());
        params.put("_processor_id", product.getProcessorId());
        params.put("_satellite_id", product.getSatellite().value());
        params.put("_site_id", product.getSiteId());
        params.put("_job_id", null);
        params.put("_full_path", product.getFullPath());
        params.put("_created_timestamp", Timestamp.valueOf(product.getCreated() != null ? product.getCreated() : LocalDateTime.now()));
        params.put("_name", product.getProductName());
        params.put("_quicklook_image", null);
        try {
            params.put("_footprint", new GeometryAdapter().unmarshal(product.getFootprint()));
        } catch (Exception e) {
            logger.warning(String.format("Cannot obtain product footprint. Reason: %s", e.getMessage()));
            params.put("_footprint", null);
        }
        params.put("_orbit_id", product.getRelativeOrbit());
        params.put("_tiles", StringUtilities.toJson(product.getTiles()));
        params.put("_orbit_type_id", product.getOrbitType() != null ? product.getOrbitType().value() : null);
        params.put("_downloader_history_id", product.getDownloadProductId() > 0 ? product.getDownloadProductId() : null);
        Map<String, Object> result = insertProductCall.execute(params);
        try {
            Map map = (Map) ((ArrayList) result.values().stream().findFirst().get()).get(0);
            Integer id = (Integer) map.get("result");
            return findProduct(id);
        } catch (Exception e) {
            return product;
        }
    }

    HighLevelProduct setArchived(HighLevelProduct product) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        jdbcTemplate.update(connection -> {
            PreparedStatement statement =
                    connection.prepareStatement("UPDATE product " +
                                                        "SET is_archived = ?, archived_timestamp = ? " +
                                                        "WHERE id = ?");
            statement.setBoolean(1, product.isArchived());
            statement.setTimestamp(2, Timestamp.valueOf(product.getArchivedTimestamp() != null ?
                                                                product.getArchivedTimestamp() : LocalDateTime.now()));
            statement.setInt(3, product.getId());
            return statement;
        });
        return findProduct(product.getId());
    }

    @Override
    protected String selectQuery() {
        return "SELECT id, site_id, processor_id, name, full_path, " +
                "created_timestamp, inserted_timestamp, is_archived, archived_timestamp," +
                "satellite_id, product_type_id, geog, tiles, orbit_type_id, downloader_history_id FROM product ";
    }

    @Override
    protected String insertQuery() {
        throw new RuntimeException("This should not be called");
    }

    @Override
    protected String updateQuery() {
        throw new RuntimeException("This should not be called");
    }

    @Override
    protected String deleteQuery() {
        return "DELETE FROM product ";
    }

    private abstract class HighLevelProductTemplate extends Template {
        @Override
        protected String baseSQL() { return selectQuery(); }

        @Override
        protected RowMapper<HighLevelProduct> rowMapper() {
            return (resultSet, i) -> {
                HighLevelProduct product = new HighLevelProduct();
                product.setId(resultSet.getInt(1));
                product.setSiteId(resultSet.getShort(2));
                product.setProcessorId(resultSet.getInt(3));
                product.setProductName(resultSet.getString(4));
                product.setFullPath(resultSet.getString(5));
                Timestamp timestamp = resultSet.getTimestamp(6);
                if (timestamp != null) {
                    product.setCreated(timestamp.toLocalDateTime());
                }
                timestamp = resultSet.getTimestamp(7);
                if (timestamp != null) {
                    product.setInserted(timestamp.toLocalDateTime());
                }
                product.setArchived(resultSet.getBoolean(8));
                timestamp = resultSet.getTimestamp(9);
                if (timestamp != null) {
                    product.setInserted(timestamp.toLocalDateTime());
                }
                product.setSatellite(new SatelliteConverter().convertToEntityAttribute(resultSet.getInt(10)));
                product.setProductType(new ProductTypeConverter().convertToEntityAttribute(resultSet.getInt(11)));
                try {
                    product.setFootprint(new GeometryAdapter().marshal(resultSet.getString(12)));
                } catch (Exception e) {
                    logger.warning("Cannot recreate footprint from database. Reason: " + e.getMessage());
                }
                Array array = resultSet.getArray(13);
                if (array != null) {
                    product.setTiles((String[]) array.getArray());
                }
                product.setOrbitType(new OrbitTypeConverter().convertToEntityAttribute(resultSet.getInt(14)));
                int value = resultSet.getInt(15);
                if (value > 0) {
                    product.setDownloadProductId(value);
                }
                return product;
            };
        }
    }
}
