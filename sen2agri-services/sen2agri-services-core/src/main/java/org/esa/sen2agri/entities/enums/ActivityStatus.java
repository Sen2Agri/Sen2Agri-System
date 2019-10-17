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

package org.esa.sen2agri.entities.enums;

import ro.cs.tao.TaoEnum;

import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;

@XmlEnum(Integer.class)
public enum ActivityStatus implements TaoEnum<Integer> {

    @XmlEnumValue("1")
    SUBMITTED(1, "Submitted"),
    @XmlEnumValue("2")
    PENDING_START(2, "Pending start"),
    @XmlEnumValue("3")
    NEEDS_INPUT(3, "Needs input"),
    @XmlEnumValue("4")
    RUNNING(4, "Running"),
    @XmlEnumValue("5")
    PAUSED(5, "Paused"),
    @XmlEnumValue("6")
    FINISHED(6, "Completed"),
    @XmlEnumValue("7")
    CANCELLED(7, "Cancelled"),
    @XmlEnumValue("8")
    ERROR(8, "Error");

    private final int value;
    private final String description;

    ActivityStatus(int value, String description) {
        this.value = value;
        this.description = description;
    }

    @Override
    public String friendlyName() { return this.description; }

    @Override
    public Integer value() { return this.value; }

}
