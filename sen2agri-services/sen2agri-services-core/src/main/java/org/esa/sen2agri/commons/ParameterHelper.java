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

package org.esa.sen2agri.commons;

import org.esa.sen2agri.entities.Satellite;

public class ParameterHelper {

    public static String getStartDateParamName(Satellite satellite) {
        switch (satellite) {
            case Sentinel1:
            case Sentinel2:
                return "beginPosition";
            case Landsat8:
                return "sensingStart";
            default:
                return "startDate";
        }
    }

    public static String getEndDateParamName(Satellite satellite) {
        switch (satellite) {
            case Sentinel1:
            case Sentinel2:
                return "endPosition";
            case Landsat8:
                return "sensingEnd";
            default:
                return "endDate";
        }
    }

    public static String getFootprintParamName(Satellite satellite) {
        switch (satellite) {
            case Sentinel1:
            case Sentinel2:
                return "footprint";
            case Landsat8:
            default:
                return "footprint";
        }
    }

    public static String getTileParamName(Satellite satellite) {
        switch (satellite) {
            case Sentinel1:
                return "relativeOrbitNumber";
            case Sentinel2:
                return "tileId";
            case Landsat8:
                return "row_path";
            default:
                return null;
        }
    }
}
