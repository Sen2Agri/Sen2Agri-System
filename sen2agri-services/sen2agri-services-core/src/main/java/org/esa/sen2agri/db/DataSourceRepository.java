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

import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

/**
 * @author Cosmin Cara
 */
@Repository
@Qualifier(value = "dataSourceRepository")
@Transactional
public interface DataSourceRepository extends JpaRepository<DataSourceConfiguration, Short> {

    @Query(value = "SELECT DISTINCT ON (site_id, satellite_id, scope, enabled) id, site_id, satellite_id, name, scope, username, passwrd, fetch_mode, " +
            "max_retries, retry_interval_minutes, download_path, specific_params,max_connections, local_root, enabled, secondary_datasource_id " +
            "FROM datasource ORDER BY site_id, satellite_id, scope, enabled, name DESC",
            nativeQuery = true)
    List<DataSourceConfiguration> getConfigurations();
}
