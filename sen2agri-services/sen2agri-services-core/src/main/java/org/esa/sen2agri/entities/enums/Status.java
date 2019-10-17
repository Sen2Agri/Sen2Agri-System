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
public enum Status implements TaoEnum<Short> {
    @XmlEnumValue("1")
    DOWNLOADING((short) 1, "Downloading"),
    @XmlEnumValue("2")
    DOWNLOADED((short) 2, "Downloaded"),
    @XmlEnumValue("3")
    FAILED((short) 3, "Download failed"),
    @XmlEnumValue("4")
    ABORTED((short) 4, "Download aborted"),
    @XmlEnumValue("41")
    IGNORED((short) 41, "Download ignored"),
    @XmlEnumValue("5")
    PROCESSED((short) 5, "Acquisition processed"),
    @XmlEnumValue("6")
    PROCESSING_FAILED((short) 6, "Acquisition processing failed"),
    @XmlEnumValue("7")
    PROCESSING((short) 7, "Acquisition processing");

    private final short value;
    private final String description;

    Status(short value, String description) {
        this.value = value;
        this.description = description;
    }


    @Override
    public String friendlyName() { return this.description; }

    @Override
    public Short value() { return this.value; }
}
