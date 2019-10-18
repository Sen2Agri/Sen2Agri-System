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
import org.springframework.dao.DataAccessException;

import java.sql.*;
import java.util.Properties;
import java.util.logging.Logger;

/**
 * @author Cosmin Cara
 */
public class Database {

    protected static final Logger logger = Logger.getLogger(Database.class.getSimpleName());

    public static void checkProductStats() throws Exception {
        try (Connection connection = getConnection();
            PreparedStatement preparedStatement = connection.prepareStatement("SELECT table_name FROM information_schema.tables WHERE table_name = 'product_stats'")) {
            ResultSet resultSet = preparedStatement.executeQuery();
            if (!resultSet.next()) {
                logger.warning("[product_stats] table was not found and it will be created");
                String[] statements = new String[]{
                        "CREATE TABLE product_stats (product_id integer NOT NULL, " +
                                "min_value real NOT NULL, " +
                                "max_value real NOT NULL, " +
                                "mean_value real NOT NULL, " +
                                "std_dev real NOT NULL, " +
                                "histogram integer[256] NULL, " +
                                "CONSTRAINT pk_product_details PRIMARY KEY (product_id), " +
                                "CONSTRAINT fk_product FOREIGN KEY (product_id) REFERENCES product (id) MATCH SIMPLE " +
                                "ON UPDATE NO ACTION ON DELETE NO ACTION)"
                };
                for (String statement : statements) {
                    try (PreparedStatement pStatement = connection.prepareStatement(statement)){
                        pStatement.execute();
                    } catch (DataAccessException ex) {
                        logger.severe(String.format("SQL Statement failed ('%s'): %s", statement, ex.getMessage()));
                        break;
                    }
                }
            }
            resultSet.close();
        }
    }

    public static void checkDatasource() throws Exception {
        try (Connection connection = getConnection();
             PreparedStatement preparedStatement = connection.prepareStatement("SELECT column_name FROM information_schema.columns where table_name = 'datasource' and column_name = 'secondary_datasource_id'")) {
            ResultSet resultSet = preparedStatement.executeQuery();
            if (!resultSet.next()) {
                logger.warning("Column [secondary_datasource_id] not found in table [datasource]. It will be created");
                String[] statements = new String[]{
                        "ALTER TABLE datasource ADD COLUMN secondary_datasource_id integer NULL",
                        "ALTER TABLE datasource ADD CONSTRAINT fk_datasource_datasource FOREIGN KEY (secondary_datasource_id) " +
                                "REFERENCES datasource (id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE NO ACTION",
                        "WITH rows AS (SELECT id as other FROM datasource WHERE satellite_id = 1 AND name = 'Scientific Data Hub' AND scope > 1)" +
                                "UPDATE datasource SET secondary_datasource_id = other FROM rows WHERE id != other AND satellite_id = 1",
                        "WITH rows AS (SELECT id as other FROM datasource WHERE satellite_id = 2 AND name = 'USGS' AND scope > 1)" +
                                "UPDATE datasource SET secondary_datasource_id = other FROM rows WHERE id != other AND satellite_id = 2",
                        "WITH rows AS (SELECT id as other FROM datasource WHERE satellite_id = 3 AND name = 'Scientific Data Hub' AND scope > 1)" +
                                "UPDATE datasource SET secondary_datasource_id = other FROM rows WHERE id != other AND satellite_id = 3"
                };
                for (String statement : statements) {
                    try (PreparedStatement pStatement = connection.prepareStatement(statement)) {
                        pStatement.execute();
                    } catch (DataAccessException ex) {
                        logger.severe(String.format("SQL Statement failed ('%s'): %s", statement, ex.getMessage()));
                        break;
                    }
                }
            }
            resultSet.close();
        }
    }

    public static void checkConfig() throws Exception {
        try (Connection connection = getConnection();
             PreparedStatement preparedStatement = connection.prepareStatement("SELECT key FROM config WHERE key = 'scheduled.reports.enabled'")) {
            ResultSet resultSet = preparedStatement.executeQuery();
            if (!resultSet.next()) {
                logger.fine("Configuration keys from services need an update");
                String[] statements = new String[] {
                        "UPDATE config SET key = 'processor.l2s1.extract.histogram' WHERE key = 'l2.extract.histogram';",
                        "UPDATE config SET key = 'processor.l2s1.gpt.tile.cache.size' WHERE key = 's1.preprocessing.gpt.tile.cache.size';",
                        "UPDATE config SET key = 'processor.l2s1.parallelism' WHERE key = 's1.preprocessing.parallelism';",
                        "UPDATE config SET key = 'processor.l2s1.gpt.parallelism' WHERE key = 's1.preprocessing.gpt.parallelism';",
                        "UPDATE config SET key = 'processor.l2s1.compute.amplitude' WHERE key = 's1.amplitude.enabled';",
                        "UPDATE config SET key = 'processor.l2s1.compute.coherence' WHERE key = 's1.coherence.enabled';",
                        "UPDATE config SET key = 'processor.l2s1.crop.output' WHERE key = 's1.preprocessing.crop.output';",
                        "UPDATE config SET key = 'processor.l2s1.keep.intermediate' WHERE key = 's1.preprocessing.keep.intermediate';",
                        "UPDATE config SET key = 'processor.l2s1.crop.nodata' WHERE key = 's1.preprocessing.crop.nodata';",
                        "UPDATE config SET key = 'processor.l2s1.output.format' WHERE key = 's1.preprocessing.output.format';",
                        "UPDATE config SET key = 'processor.l2s1.output.extension' WHERE key = 's1.preprocessing.output.extension';",
                        "UPDATE config SET key = 'processor.l2s1.parallel.steps.enabled' WHERE key = 's1.preprocessing.parallel.steps.enabled';",
                        "UPDATE config SET key = 'processor.l2s1.projection' WHERE key = 's1.preprocessing.projection';",
                        "UPDATE config SET key = 'processor.l2s1.pixel.spacing' WHERE key = 's1.preprocessing.pixel.spacing';",
                        "UPDATE config SET key = 'processor.l2s1.copy.locally' WHERE key = 's1.preprocessing.copy.locally';",
                        "UPDATE config SET key = 'processor.l2s1.overwrite.existing' WHERE key = 's1.preprocessing.overwrite.existing';",
                        "UPDATE config SET key = 'processor.l2s1.path' WHERE key = 's1.preprocessing.path';",
                        "UPDATE config SET key = 'processor.l2s1.parallelism' WHERE key = 's1.preprocessing.parallelism';",
                        "UPDATE config SET key = 'processor.l2s1.enabled' WHERE key = 's1.preprocessing.enabled';",
                        "UPDATE config SET key = 'scheduled.reports.enabled' WHERE key = 'reports.enabled';",
                        "UPDATE config SET key = 'scheduled.reports.interval' WHERE key = 'reports.interval';",
                        "UPDATE config SET key = 'processor.l2s1.work.dir' WHERE key = 's1.preprocessing.work.dir';",
                        "UPDATE config SET key = 'processor.l2s1.temporal.offset' WHERE key = 's1.preprocessing.temporal.offset';",
                        "UPDATE config SET key = 'processor.l2s1.step.timeout' WHERE key = 's1.preporcessing.step.timeout';",
                        "UPDATE config SET key = 'processor.l2s1.min.intersection' WHERE key = 's1.preprocessing.min.intersection';",
                        "UPDATE config SET key = 'processor.l2s1.min.memory' WHERE key = 's1.preprocessing.min.memory';",
                        "UPDATE config SET key = 'processor.l2s1.polarisations' WHERE key = 's1.preprocessing.polarisations';",
                        "UPDATE config SET key = 'processor.l2s1.resolve.links' WHERE key = 's1.preprocessing.resolve.links';",
                        "UPDATE config SET key = 'processor.l2s1.process.newest' WHERE key = 's1.process.newest'"
                };
                for (String statement : statements) {
                    try (PreparedStatement pStatement = connection.prepareStatement(statement)) {
                        pStatement.execute();
                    } catch (DataAccessException ex) {
                        logger.severe(String.format("SQL Statement failed ('%s'): %s", statement, ex.getMessage()));
                        break;
                    }
                }
            }
            resultSet.close();
        }
    }

    protected static Connection getConnection() throws SQLException {
        String dbUrl = Config.getProperty("spring.datasource.url");
        String user = Config.getProperty("spring.datasource.username");
        String pwd = Config.getProperty("spring.datasource.password");
        if (dbUrl == null || user == null || pwd == null) {
            throw new IllegalArgumentException("Cannot read database connection parameters");
        }
        Properties props = new Properties();
        props.setProperty("user", user);
        props.setProperty("password", pwd);
        props.setProperty("ssl", "false");
        return DriverManager.getConnection(dbUrl, props);
    }
}
