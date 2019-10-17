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
package org.esa.sen2agri.entities.converters;

import org.esa.sen2agri.entities.enums.Status;
import ro.cs.tao.EnumUtils;

import javax.persistence.AttributeConverter;

/**
 * @author Cosmin Cara
 */
public class StatusConverter implements AttributeConverter<Status, Short> {
    @Override
    public Short convertToDatabaseColumn(Status status) {
        return status != null ? status.value() : null;
    }

    @Override
    public Status convertToEntityAttribute(Short integer) {
        return integer != null ? EnumUtils.getEnumConstantByValue(Status.class, integer) : null;
    }
}
