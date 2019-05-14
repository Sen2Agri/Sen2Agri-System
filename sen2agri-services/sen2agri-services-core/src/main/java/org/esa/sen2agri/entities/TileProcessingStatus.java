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

package org.esa.sen2agri.entities;

import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;

@XmlEnum(Integer.class)
public enum TileProcessingStatus {
    @XmlEnumValue("1")
    PROCESSING(1),
    @XmlEnumValue("2")
    FAILED(2),
    @XmlEnumValue("3")
    DONE(3);

    private final int value;
    TileProcessingStatus(int value) { this.value = value; }

    @Override
    public String toString()
    {
        return String.valueOf(this.value);
    }

    public int value() { return value; }

    /**
     * Retrieve string enum token corresponding to the integer identifier
     * @param value the integer value identifier
     * @return the string token corresponding to the integer identifier
     */
    public static TileProcessingStatus getEnumConstantByValue(final int value) {
        for (TileProcessingStatus type : values()) {
            if (value == type.value) {
                // return the name of the enum constant having the given value
                return type;
            }
        }
        return null;
    }
}
