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

import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;

import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

public abstract class NonMappedRepository<T> {
    protected PersistenceManager persistenceManager;
    protected final Logger logger = Logger.getLogger(NonMappedEntitiesRepository.class.getName());

    NonMappedRepository(PersistenceManager persistenceManager) {
        this.persistenceManager = persistenceManager;
    }

    protected abstract String selectQuery();
    protected abstract String insertQuery();
    protected abstract String updateQuery();
    protected abstract String deleteQuery();

    protected abstract class Template {
        protected abstract String baseSQL();
        protected abstract String conditionsSQL();
        protected abstract void mapParameters(PreparedStatement statement) throws SQLException;
        protected abstract RowMapper<T> rowMapper();

        T single() {
            List<T> results = list();
            return results != null && results.size() == 1 ? results.get(0) : null;
        }

        public List<T> list() {
            DataSource dataSource = persistenceManager.getDataSource();
            JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
            try (Connection conn = dataSource.getConnection()) {
                return jdbcTemplate.query(
                        connection -> {
                            PreparedStatement statement = connection.prepareStatement(baseSQL() + " " + conditionsSQL());
                            mapParameters(statement);
                            return statement;
                        }, rowMapper());
            } catch (Exception ex) {
                Logger.getLogger(getClass().getName()).warning(ex.getMessage());
                return new ArrayList<>();
            }
        }

        public int delete() {
            DataSource dataSource = persistenceManager.getDataSource();
            JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
            try (Connection connection = dataSource.getConnection()) {
                return jdbcTemplate.update(deleteQuery() + " " + conditionsSQL(),
                                           this::mapParameters);
            } catch (SQLException ex) {
                Logger.getLogger(getClass().getName()).warning(ex.getMessage());
                return 0;
            }
        }
    }
}
