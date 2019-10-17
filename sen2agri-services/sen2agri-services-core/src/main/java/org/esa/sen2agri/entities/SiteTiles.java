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

import org.esa.sen2agri.entities.enums.Satellite;
import org.hibernate.annotations.Type;

import java.io.Serializable;

/**
 * @author Cosmin Cara
 */
public class SiteTiles implements Serializable {

    private short siteId;
    private Satellite satellite;
    private String[] tiles;

    public short getSiteId() { return siteId; }
    public void setSiteId(short site) { this.siteId = site; }

    public Satellite getSatellite() { return satellite; }
    public void setSatellite(Satellite value) { this.satellite = value; }

    @Type(type = "string-array")
    public String[] getTiles() {
        return tiles;
    }
    public void setTiles(String[] tiles) {
        this.tiles = tiles;
    }

    @Override
    public int hashCode() {
        return super.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj);
    }
}
