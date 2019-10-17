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
package org.esa.sen2agri.web;

import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.services.DataSourceService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import ro.cs.tao.component.validation.ValidationException;
import ro.cs.tao.services.commons.ControllerBase;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Controller
@RequestMapping("/downloader/sources")
public class DatasourceController  extends ControllerBase {

    @Autowired
    private DataSourceService dataSourceService;

    @RequestMapping(value = "/", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<DataSourceConfiguration>> getConfigurations() {
        final List<DataSourceConfiguration> list = dataSourceService.getDataSourceConfigurations();
        if (list == null || list.isEmpty()) {
            return new ResponseEntity<>(HttpStatus.OK);
        }
        return new ResponseEntity<>(list, HttpStatus.OK);
    }

    @RequestMapping(value = "/{satellite}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<DataSourceConfiguration>> getConfigurations(@PathVariable("satellite") short satelliteId) {
        final List<DataSourceConfiguration> list =
                dataSourceService.getDataSourceConfigurations(new SatelliteConverter().convertToEntityAttribute(satelliteId));
        if (list == null || list.isEmpty()) {
            return new ResponseEntity<>(HttpStatus.OK);
        }
        return new ResponseEntity<>(list, HttpStatus.OK);
    }

    @RequestMapping(value = "/{satellite}/{name}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<DataSourceConfiguration> getConfiguration(@PathVariable("satellite") short satelliteId,
                                                                    @PathVariable("name") String name) {
        DataSourceConfiguration entity =
                dataSourceService.getDataSourceConfiguration(new SatelliteConverter().convertToEntityAttribute(satelliteId),
                                                             name);
        if (entity == null) {
            return new ResponseEntity<>(HttpStatus.OK);
        }
        return new ResponseEntity<>(entity, HttpStatus.OK);
    }

    @RequestMapping(value = "/{satellite}/{name}", method = RequestMethod.POST, produces = "application/json")
    public ResponseEntity<?> updateConfiguration(@PathVariable("satellite") short satelliteId,
                                                 @PathVariable("name") String name,
                                                 @RequestBody DataSourceConfiguration entity) {
        if (entity == null || entity.getId() <= 0 ||
                !Objects.equals(name, entity.getDataSourceName()) ||
                satelliteId != entity.getSatellite().value()) {
            return new ResponseEntity<>(entity, HttpStatus.BAD_REQUEST);
        }
        final ResponseEntity<?> validationResponse = validate(entity);
        if (validationResponse.getStatusCode() == HttpStatus.OK) {
            dataSourceService.updateDataSourceConfiguration(entity);
            return new ResponseEntity<>(entity, HttpStatus.OK);
        } else {
            return validationResponse;
        }
    }

    private ResponseEntity<?> validate(DataSourceConfiguration entity) {
        try {
            dataSourceService.validate(entity);
            return new ResponseEntity<>(HttpStatus.OK);
        } catch (ValidationException ex) {
            List<String> errors = new ArrayList<>();
            String message = ex.getMessage();
            if (message != null && !message.isEmpty()) {
                errors.add(message);
            }
            final Map<String, Object> info = ex.getAdditionalInfo();
            if (info != null) {
                if (info.values().stream().allMatch(Objects::isNull)) {
                    errors.addAll(info.keySet());
                } else {
                    errors.addAll(info.entrySet().stream()
                                          .map(e -> e.getKey() + (e.getValue() != null ? ":" + e.getValue() : ""))
                                          .collect(Collectors.toSet()));
                }
            }
            return new ResponseEntity<>(errors, HttpStatus.NOT_ACCEPTABLE);
        }
    }
}
