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

import org.esa.sen2agri.entities.ProductCount;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.entities.SiteTiles;
import org.springframework.jdbc.core.BatchPreparedStatementSetter;
import org.springframework.jdbc.core.JdbcTemplate;

import javax.sql.DataSource;
import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.time.LocalDateTime;
import java.util.List;
import java.util.Objects;

/**
 * Repository-like class for handling entities not mapped in the orm.xml file.
 *
 * @author Cosmin Cara
 */
class NonMappedEntitiesRepository {

    private PersistenceManager persistenceManager;

    NonMappedEntitiesRepository(PersistenceManager persistenceManager) {
        this.persistenceManager = persistenceManager;
    }

    SiteTiles getSiteTiles(short siteId, short satelliteId) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        SiteTiles siteTiles = new SiteTiles();
        jdbcTemplate.query(
                connection -> {
                    PreparedStatement statement =
                            connection.prepareStatement("SELECT site_id, satellite_id, tiles FROM site_tiles " +
                                    "WHERE site_id = ? AND satellite_id = ?");
                    statement.setShort(1, siteId);
                    statement.setShort(2, satelliteId);
                    return statement;
                },
                (resultSet, i) -> {
                    siteTiles.setSiteId(resultSet.getShort(1));
                    siteTiles.setSatellite(Enum.valueOf(Satellite.class,
                            Objects.requireNonNull(Satellite.getEnumConstantNameByValue(resultSet.getShort(2)))));
                    siteTiles.setTiles((String[]) resultSet.getArray(3).getArray());
                    return siteTiles;
                });
        return siteTiles;
    }

    List<ProductCount> getEstimatedProductCount(int siteId, int satelliteId) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        return jdbcTemplate.query(
                connection -> {
                    PreparedStatement statement =
                            connection.prepareStatement("SELECT site_id, satellite_id, start_date, end_date, product_count FROM downloader_count " +
                                    "WHERE site_id = ? AND satellite_id = ?");
                    statement.setInt(1, siteId);
                    statement.setInt(2, satelliteId);
                    return statement;
                },
                (resultSet, i) -> {
                    ProductCount productCount = new ProductCount();
                    productCount.setSiteId(resultSet.getShort(1));
                    productCount.setSatellite(Enum.valueOf(Satellite.class,
                            Objects.requireNonNull(Satellite.getEnumConstantNameByValue(resultSet.getShort(2)))));
                    productCount.setStartDate(resultSet.getDate(3).toLocalDate());
                    productCount.setEndDate(resultSet.getDate(4).toLocalDate());
                    productCount.setCount(resultSet.getInt(5));
                    return productCount;
                });
    }

    void save(ProductCount productCount) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        jdbcTemplate.batchUpdate("WITH upsert AS ( UPDATE downloader_count SET product_count = ?, last_updated = ? " +
                        "WHERE site_id = ? AND satellite_id = ? AND start_date = ? AND end_date = ? RETURNING *) " +
                        "INSERT INTO downloader_count (site_id, satellite_id, start_date, end_date, product_count) " +
                        "SELECT ?, ?, ?, ?, ? WHERE NOT EXISTS (SELECT * FROM upsert);",
                new BatchPreparedStatementSetter() {
                    @Override
                    public void setValues(PreparedStatement preparedStatement, int row) throws SQLException {
                        preparedStatement.setInt(1, productCount.getCount());
                        preparedStatement.setTimestamp(2, Timestamp.valueOf(LocalDateTime.now()));
                        preparedStatement.setShort(3, productCount.getSiteId());
                        preparedStatement.setShort(4, productCount.getSatellite().value());
                        preparedStatement.setDate(5, Date.valueOf(productCount.getStartDate()));
                        preparedStatement.setDate(6, Date.valueOf(productCount.getEndDate()));
                        preparedStatement.setShort(7, productCount.getSiteId());
                        preparedStatement.setShort(8, productCount.getSatellite().value());
                        preparedStatement.setDate(9, Date.valueOf(productCount.getStartDate()));
                        preparedStatement.setDate(10, Date.valueOf(productCount.getEndDate()));
                        preparedStatement.setInt(11, productCount.getCount());
                    }

                    @Override
                    public int getBatchSize() {
                        return 1;
                    }
                });
    }
}
