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

import org.esa.sen2agri.entities.DownloadProductTile;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.entities.converters.TileProcessingStatusConverter;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.entities.enums.TileProcessingStatus;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Sort;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;

import javax.sql.DataSource;
import java.sql.*;
import java.time.LocalDate;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class DownloadProductTileRepository extends NonMappedRepository<DownloadProductTile> {

    DownloadProductTileRepository(PersistenceManager persistenceManager) {
        super(persistenceManager);
    }

    List<DownloadProductTile> findBySite(int siteId) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return "JOIN downloader_history dh ON dh.id = th.downloader_history_id " +
                        "WHERE dh.site_id = ?";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
            }
        }.list();
    }

    List<DownloadProductTile> findBySiteAndSatellite(int siteId, Satellite satellite) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return "JOIN downloader_history dh ON dh.id = th.downloader_history_id " +
                        "WHERE dh.site_id = ? AND dh.satellite_id = ?";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
                statement.setInt(2, satellite.value());
            }
        }.list();
    }

    DownloadProductTile findOne(int productId, String tileId) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE th.downloader_history_id = ? AND th.tile_id = ?";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, productId);
                statement.setString(2, tileId);
            }
        }.single();
    }

    List<DownloadProductTile> findByProductId(int productId) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return "WHERE th.downloader_history_id = ?";
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, productId);
            }
        }.list();
    }

    List<DownloadProductTile> findAll(PageRequest pageRequest) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("ORDER BY %s OFFSET %s LIMIT %s,",
                                     pageRequest.getSort().stream()
                                             .map(o -> "th." + o.getProperty() + " " + o.getDirection().name())
                                             .collect(Collectors.joining(",")),
                                     pageRequest.getOffset(),
                                     pageRequest.getPageSize());
            }
            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    List<DownloadProductTile> findAll(Sort sort) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                return String.format("ORDER BY %s",
                                     sort.stream()
                                             .map(o -> "th." + o.getProperty() + " " + o.getDirection().name())
                                             .collect(Collectors.toList()));
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    List<DownloadProductTile> findByStatus(TileProcessingStatus... statuses) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                if (statuses == null || statuses.length == 0) {
                    return "ORDER BY th.status_timestamp";
                } else {
                    return String.format("WHERE th.status_id IN (%s) ORDER BY dh.status_timestamp",
                                         Arrays.stream(statuses)
                                                 .map(s -> String.valueOf(s.value()))
                                                 .collect(Collectors.joining(",")));
                }
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
            }
        }.list();
    }

    List<DownloadProductTile> findBySiteAndStatus(int siteId, TileProcessingStatus... statuses) {
        return new DownloadProductTileTemplate() {
            @Override
            protected String conditionsSQL() {
                if (statuses == null || statuses.length == 0) {
                    return "JOIN downloader_history dh ON dh.id = th.downloader_history_id " +
                            "WHERE dh.site_id = ? ORDER BY th.status_timestamp";
                } else {
                    return String.format("JOIN downloader_history dh ON dh.id = th.downloader_history_id " +
                                                 "WHERE dh.side_id = ? AND th.status_id IN (%s) ORDER BY dh.status_timestamp",
                                         Arrays.stream(statuses)
                                                 .map(s -> String.valueOf(s.value()))
                                                 .collect(Collectors.joining(",")));
                }
            }

            @Override
            protected void mapParameters(PreparedStatement statement) throws SQLException {
                statement.setInt(1, siteId);
            }
        }.list();
    }

    Integer countByStatus(int siteId, Satellite satellite, TileProcessingStatus status, LocalDate start, LocalDate end) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        return jdbcTemplate.query(
                connection -> {
                    PreparedStatement statement =
                            connection.prepareStatement("SELECT COUNT(th.*) FROM l1_tile_history th " +
                                                                "JOIN downloader_history dh on dh.id = th.downloader_history_id " +
                                                                "WHERE dh.site_id = ? AND dh.product_date BETWEEN ? AND ? " +
                                                                "AND th.status_id = ? AND th.satellite_id = ? " +
                                                                "ORDER BY dh.product_date");
                    statement.setInt(1, siteId);
                    statement.setDate(2, Date.valueOf(start));
                    statement.setDate(3, Date.valueOf(end));
                    statement.setInt(4, status.value());
                    statement.setInt(5, satellite.value());
                    return statement;
                },
                (resultSet, i) -> resultSet.getInt(1)).stream().findFirst().orElse(0);
    }

    DownloadProductTile save(DownloadProductTile tile) {
        DataSource dataSource = persistenceManager.getDataSource();
        JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
        final boolean isUpdateQuery = tile.getTileId() != null && findOne(tile.getDownloadProductId(), tile.getTileId()) != null;
        int result = jdbcTemplate.update(connection -> {
            PreparedStatement statement;
            if (isUpdateQuery) {
                statement = connection.prepareStatement(updateQuery());
                statement.setInt(1, tile.getStatus().value());
                statement.setTimestamp(2, Timestamp.valueOf(tile.getStatusTimestamp()));
                statement.setString(3, tile.getFailReason());
                Integer value = tile.getCloudCoverage();
                if (value != null) {
                    statement.setInt(4, value);
                } else {
                    statement.setNull(4, Types.INTEGER);
                }
                value = tile.getSnowCoverage();
                if (value != null) {
                    statement.setInt(5, value);
                } else {
                    statement.setNull(5, Types.INTEGER);
                }
                statement.setInt(6, tile.getSatellite().value());
                statement.setInt(7, tile.getOrbitId());
                statement.setInt(8, tile.getDownloadProductId());
                statement.setString(9, tile.getTileId());
            } else {
                statement = connection.prepareStatement(insertQuery());
                statement.setInt(1, tile.getSatellite().value());
                statement.setInt(2, tile.getOrbitId());
                statement.setString(3, tile.getTileId());
                statement.setInt(4, tile.getDownloadProductId());
                statement.setInt(5, tile.getStatus().value());
                statement.setTimestamp(6, Timestamp.valueOf(tile.getStatusTimestamp()));
                statement.setInt(7, tile.getRetryCount());
                statement.setString(8, tile.getFailReason());
                Integer value = tile.getCloudCoverage();
                if (value != null) {
                    statement.setInt(9, value);
                } else {
                    statement.setNull(9, Types.INTEGER);
                }
                value = tile.getSnowCoverage();
                if (value != null) {
                    statement.setInt(10, value);
                } else {
                    statement.setNull(10, Types.INTEGER);
                }
            }
            return statement;
        });
        return result == 1 ? tile : null;
    }

    @Override
    protected String selectQuery() {
        return "SELECT th.satellite_id, th.orbit_id, th.tile_id, th.downloader_history_id, th.status_id, th.status_timestamp," +
                "th.retry_count, th.failed_reason, th.cloud_coverage, th.snow_coverage from l1_tile_history th ";
    }

    @Override
    protected String insertQuery() {
        return "INSERT INTO l1_tile_history (satellite_id, orbit_id, tile_id, downloader_history_id, status_id, status_timestamp," +
                "retry_count, failed_reason, cloud_coverage, snow_coverage) VALUES (?,?,?,?,?,?,?,?,?,?)";
    }

    @Override
    protected String updateQuery() {
        return "UPDATE l1_tile_history SET status_id = ?, status_timestamp = ?, failed_reason = ?, cloud_coverage = ?," +
                "snow_coverage = ?, satellite_id = ?, orbit_id = ? WHERE downloader_history_id = ? AND tile_id = ? ";
    }

    @Override
    protected String deleteQuery() {
        return "DELETE FROM l1_tile_history ";
    }

    private abstract class DownloadProductTileTemplate extends Template {
        @Override
        protected String baseSQL() { return selectQuery(); }

        @Override
        protected RowMapper<DownloadProductTile> rowMapper() {
            return (resultSet, rowNum) -> {
                DownloadProductTile tile = new DownloadProductTile();
                tile.setSatellite(new SatelliteConverter().convertToEntityAttribute(resultSet.getShort(1)));
                tile.setOrbitId(resultSet.getInt(2));
                tile.setTileId(resultSet.getString(3));
                tile.setDownloadProductId(resultSet.getInt(4));
                tile.setStatus(new TileProcessingStatusConverter().convertToEntityAttribute(resultSet.getInt(5)));
                Timestamp timestamp = resultSet.getTimestamp(6);
                if (timestamp != null) {
                    tile.setStatusTimestamp(timestamp.toLocalDateTime());
                }
                tile.setRetryCount(resultSet.getInt(7));
                tile.setFailReason(resultSet.getString(8));
                Object value = resultSet.getObject(9);
                if (value != null) {
                    tile.setCloudCoverage(resultSet.getInt(9));
                }
                value = resultSet.getObject(10);
                if (value != null) {
                    tile.setSnowCoverage(resultSet.getInt(10));
                }
                return tile;
            };
        }
    }
}
