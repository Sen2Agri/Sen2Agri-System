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
package org.esa.sen2agri.services.internal;

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.services.DataSourceService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import ro.cs.tao.component.validation.ValidationException;
import ro.cs.tao.datasource.DataSourceManager;
import ro.cs.tao.datasource.remote.FetchMode;

import java.nio.file.InvalidPathException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Service("dataSourceService")
public class DataSourceServiceImpl implements DataSourceService {

    @Autowired
    private PersistenceManager persistenceManager;

    @Override
    public List<Satellite> getSupportedSensors() {
        return DataSourceManager.getInstance().getSupportedSensors().stream()
                .map(Satellite::valueOf).collect(Collectors.toList());
    }

    @Override
    public List<DataSourceConfiguration> getDataSourceConfigurations() {
        return persistenceManager.getDataSourceConfigurations();
    }

    @Override
    public List<DataSourceConfiguration> getDataSourceConfigurations(Satellite satellite) {
        final List<DataSourceConfiguration> list = persistenceManager.getDataSourceConfigurations();
        if (list != null) {
            return list.stream().filter(ds -> satellite.equals(ds.getSatellite())).collect(Collectors.toList());
        } else {
            return null;
        }
    }

    @Override
    public DataSourceConfiguration getDataSourceConfiguration(Satellite satellite, String dataSourceName) {
        final List<DataSourceConfiguration> list = persistenceManager.getDataSourceConfigurations();
        if (list != null) {
            return list.stream().filter(ds -> satellite.equals(ds.getSatellite()) &&
                    dataSourceName.equals(ds.getDataSourceName()))
                    .findFirst().orElse(null);
        } else {
            return null;
        }
    }

    @Override
    public void updateDataSourceConfiguration(DataSourceConfiguration configuration) {
        persistenceManager.save(configuration);
    }

    @Override
    public void validate(DataSourceConfiguration configuration) throws ValidationException {
        List<String> errors = new ArrayList<>();
        if (configuration == null) {
            throw new ValidationException("Entity cannot be null");
        } else {
            try {
                validateFields(configuration, errors);
            } catch (Throwable t) {
                errors.add(t.getMessage());
            }
        }
        if (errors.size() > 0) {
            ValidationException ex = new ValidationException("Entity has validation errors");
            errors.forEach(e -> ex.addAdditionalInfo(e, null));
            throw ex;
        }
    }

    @Override
    public void setSensorStatus(Satellite satellite, short siteId, boolean enabled) {
        Config.setSetting(siteId,
                          String.format(ConfigurationKeys.SENSOR_STATE, satellite.friendlyName().toLowerCase()),
                          Boolean.toString(enabled));
    }

    @Override
    public boolean getSensorStatus(Satellite satellite, short siteId) {
        return Config.isFeatureEnabled(siteId, String.format(ConfigurationKeys.SENSOR_STATE,
                satellite.friendlyName().toLowerCase()));
    }

    private void validateFields(DataSourceConfiguration entity, List<String> errors) {
        if (entity.getId() <= 0) {
            errors.add("[id] cannot be negative or zero");
        }
        String value = entity.getDataSourceName();
        if (value == null || value.trim().isEmpty()) {
            errors.add("[dataSourceName] cannot be empty");
        }
        if (entity.getSatellite() == null) {
            errors.add("[satellite] cannot be empty");
        }
        value = entity.getDownloadPath();
        if (value == null || value.trim().isEmpty()) {
            errors.add("[downloadPath] cannot be empty");
        } else {
            try {
                Paths.get(value);
            } catch (InvalidPathException ex) {
                errors.add("[downloadPath] value is invalid");
            }
        }
        value = entity.getLocalArchivePath();
        if (value != null) {
            try {
                Paths.get(value);
            } catch (InvalidPathException ex) {
                errors.add("[localArchivePath] value is invalid");
            }
            if (entity.getFetchMode() != FetchMode.SYMLINK && entity.getFetchMode() != FetchMode.COPY) {
                errors.add(String.format("[fetchMode] when 'localArchivePath' is set, this has to be either %s or %s",
                                         FetchMode.COPY.value(), FetchMode.SYMLINK.value()));
            }
        }
        if (entity.getFetchMode() == null) {
            errors.add("[fetchMode] cannot be empty");
        }
        if (entity.getMaxRetries() < 0) {
            errors.add("[maxRetries] cannot be negative");
        }
        if (entity.getMaxConnections() <= 0) {
            errors.add("[maxRetries] cannot be negative or zero");
        }
    }
}
