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
package org.esa.sen2agri.web.beans;

import ro.cs.tao.datasource.param.DataSourceParameter;

import java.util.Map;

/**
 * @author Cosmin Cara
 */
public class DataSourceInstance {
    private String sensor;
    private String dataSourceName;
    private Map<String, DataSourceParameter> parameters;

    public DataSourceInstance() {
    }

    public DataSourceInstance(String sensor, String dataSourceName, Map<String, DataSourceParameter> parameters) {
        this.sensor = sensor;
        this.dataSourceName = dataSourceName;
        this.parameters = parameters;
    }

    public String getSensor() {
        return sensor;
    }

    public void setSensor(String sensor) {
        this.sensor = sensor;
    }

    public String getDataSourceName() {
        return dataSourceName;
    }

    public void setDataSourceName(String dataSourceName) {
        this.dataSourceName = dataSourceName;
    }

    public Map<String, DataSourceParameter> getParameters() {
        return parameters;
    }

    public void setParameters(Map<String, DataSourceParameter> parameters) {
        this.parameters = parameters;
    }
}
