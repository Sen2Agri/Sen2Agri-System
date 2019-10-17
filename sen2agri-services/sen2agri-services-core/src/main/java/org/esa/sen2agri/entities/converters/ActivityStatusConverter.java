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

import org.esa.sen2agri.entities.enums.ActivityStatus;
import ro.cs.tao.EnumUtils;

import javax.persistence.AttributeConverter;

public class ActivityStatusConverter implements AttributeConverter<ActivityStatus, Integer> {
    @Override
    public Integer convertToDatabaseColumn(ActivityStatus activityStatus) {
        return activityStatus != null ? activityStatus.value() : null;
    }

    @Override
    public ActivityStatus convertToEntityAttribute(Integer integer) {
        return integer != null ? EnumUtils.getEnumConstantByValue(ActivityStatus.class, integer) : null;
    }
}
