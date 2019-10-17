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
package org.esa.sen2agri.services;

import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.enums.Satellite;
import ro.cs.tao.component.validation.ValidationException;

import java.util.List;

/**
 * @author Cosmin Cara
 */
public interface DataSourceService {
    /**
     * Retrieves the list of supported satellites.
     */
    List<Satellite> getSupportedSensors();
    /**
     * Retrieves the configuration of detected (available) data source modules.
     */
    List<DataSourceConfiguration> getDataSourceConfigurations();
    /**
     * Retrieves the configuration of detected (available) data source modules for the given satellite.
     */
    List<DataSourceConfiguration> getDataSourceConfigurations(Satellite satellite);
    /**
     * Retrieves the configuration of the given data source for the given satellite.
     */
    DataSourceConfiguration getDataSourceConfiguration(Satellite satellite, String dataSourceName);
    /**
     * Saves the configuration to the persistent storage.
     * @param configuration     The data source configuration to save
     */
    void updateDataSourceConfiguration(DataSourceConfiguration configuration);
    /**
     * Validates the data source parameters. Throws an exception containing all the problems encountered
     * (if encountered)
     *
     * @param configuration     The configuration to check
     * @throws ValidationException  If one or more parameters failed the check.
     */
    void validate(DataSourceConfiguration configuration) throws ValidationException;
    /**
     * Enables/disables the status of a sensor.
     * If disabled (<code>false</code>), the system should not attempt to download or process products
     * of the given sensor type (and for the given site -> if siteId=0, for all sites)
     * @param satellite The sensor
     * @param siteId    The site identifier
     * @param enabled   The status of the sensor
     */
    void setSensorStatus(Satellite satellite, short siteId, boolean enabled);

    /**
     * Get the enabled/disabled status of a sensor.
     * @param satellite The sensor
     * @param siteId    The site identifier
     * @return The status of the sensor
     */
    boolean getSensorStatus(Satellite satellite, short siteId);

}
