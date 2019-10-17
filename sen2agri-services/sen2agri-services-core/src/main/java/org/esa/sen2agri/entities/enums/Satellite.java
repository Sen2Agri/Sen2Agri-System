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

/**
 * @author Cosmin Cara
 */
@XmlEnum(Short.class)
public enum Satellite implements TaoEnum<Short> {

    @XmlEnumValue("1")
    Sentinel2((short) 1, "S2"),
    @XmlEnumValue("2")
    Landsat8((short) 2, "L8"),
    @XmlEnumValue("3")
    Sentinel1((short) 3, "S1");/*,
    @XmlEnumValue("4")
    Sentinel3((short) 4, "S3");*/

    private final short value;
    private final String description;

    Satellite(short value, String name) { this.value = value; this.description = name; }

    @Override
    public String friendlyName() { return this.description; }

    @Override
    public Short value() { return this.value; }
}
