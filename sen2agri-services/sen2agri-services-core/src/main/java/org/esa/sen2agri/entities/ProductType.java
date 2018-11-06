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

/**
 * @author Cosmin Cara
 */
@XmlEnum(Integer.class)
public enum ProductType {

    @XmlEnumValue("1")
    L2A(1, "l2a", "L2A Atmospheric correction"),
    @XmlEnumValue("2")
    L3A(2, "l3a", "L3A Composite product"),
    @XmlEnumValue("3")
    L3B_MONODATE(3, "l3b_lai_monodate", "L3B LAI mono-date product"),
    @XmlEnumValue("4")
    L3E_PHENO(4, "l3e_pheno", "L3E Pheno NDVI product"),
    @XmlEnumValue("5")
    L4A(5, "l4a", "L4A Crop mask product"),
    @XmlEnumValue("6")
    L4B(6, "l4b", "L4B Crop type product"),
    @XmlEnumValue("7")
    L1C(7, "l1c", "L1C product"),
    @XmlEnumValue("8")
    L3C_REPROCESSED(8, "l3c_lai_reproc", "L3C LAI Reprocessed product"),
    @XmlEnumValue("9")
    L3C_FITTED(9, "l3c_lai_fitted", "L3C LAI End of Season product"),
    @XmlEnumValue("10")
    L2_AMP(10, "l2-amp", "L2 SAR Amplitude product"),
    @XmlEnumValue("11")
    L2_COHE(11, "l2-cohe", "L2 SAR Coherence product");

    private final int value;
    private final String shortName;
    private final String description;

    ProductType(int value, String shortName, String description) {
        this.value = value;
        this.shortName = shortName;
        this.description = description;
    }

    @Override
    public String toString()
    {
        return String.valueOf(this.value);
    }

    public int value() { return value; }
    public String shortName() { return shortName; }
    public String description() { return description; }

    public static String getEnumConstantNameByValue(final int value) {
        for (ProductType type : values()) {
            if ((String.valueOf(value)).equals(type.toString())) {
                // return the name of the enum constant having the given value
                return type.name();
            }
        }
        return null;
    }

    public static ProductType getEnumByValue(final int value) {
        for (ProductType type : values()) {
            if ((String.valueOf(value)).equals(type.toString())) {
                // return the name of the enum constant having the given value
                return type;
            }
        }
        return null;
    }

    public static String getEnumConstantNameByShortName(final String shortName) {
        for (ProductType type : values()) {
            if (shortName.equals(type.shortName())) {
                // return the name of the enum constant having the given short name
                return type.name();
            }
        }
        return null;
    }
}
