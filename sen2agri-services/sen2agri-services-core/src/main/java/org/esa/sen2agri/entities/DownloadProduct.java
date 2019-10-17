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

import org.esa.sen2agri.entities.enums.OrbitType;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.entities.enums.Status;
import org.locationtech.jts.geom.Geometry;

import java.time.LocalDateTime;

/**
 * @author Cosmin Cara
 */
public class DownloadProduct {
    private int id;
    private short siteId;
    private Satellite satelliteId;
    private String productName;
    private String fullPath;
    private LocalDateTime timestamp;
    private Status statusId;
    private short nbRetries;
    private LocalDateTime productDate;
    private int orbitId;
    private String statusReason;
    private String[] tiles;
    private Geometry footprint;
    private OrbitType orbitType;

    public int getId() {
        return id;
    }
    public void setId(int id) {
        this.id = id;
    }

    public short getSiteId() {
        return siteId;
    }
    public void setSiteId(short siteId) {
        this.siteId = siteId;
    }

    public Satellite getSatelliteId() {
        return satelliteId;
    }
    public void setSatelliteId(Satellite satelliteId) {
        this.satelliteId = satelliteId;
    }

    public String getProductName() {
        return productName;
    }
    public void setProductName(String productName) {
        this.productName = productName;
    }

    public String getFullPath() {
        return fullPath;
    }
    public void setFullPath(String fullPath) {
        this.fullPath = fullPath;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }
    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }

    public Status getStatusId() {
        return statusId;
    }
    public void setStatusId(Status statusId) {
        this.statusId = statusId;
    }

    public short getNbRetries() {
        return nbRetries;
    }
    public void setNbRetries(short nbRetries) {
        this.nbRetries = nbRetries;
    }

    public LocalDateTime getProductDate() {
        return productDate;
    }
    public void setProductDate(LocalDateTime productDate) {
        this.productDate = productDate;
    }

    public int getOrbitId() {
        return orbitId;
    }
    public void setOrbitId(int orbitId) {
        this.orbitId = orbitId;
    }

    public String getStatusReason() { return statusReason; }
    public void setStatusReason(String statusReason) { this.statusReason = statusReason; }

    public Geometry getFootprint() { return footprint; }
    public void setFootprint(Geometry footprint) { this.footprint = footprint; }

    public String[] getTiles() { return tiles; }
    public void setTiles(String[] tiles) { this.tiles = tiles; }

    public OrbitType getOrbitType() { return orbitType; }
    public void setOrbitType(OrbitType orbitType) { this.orbitType = orbitType; }

    public DownloadProduct duplicate() {
        DownloadProduct clone = new DownloadProduct();
        clone.siteId = this.siteId;
        clone.satelliteId = this.satelliteId;
        clone.productName = this.productName;
        clone.fullPath = this.fullPath;
        clone.timestamp = this.timestamp;
        clone.statusId = this.statusId;
        clone.nbRetries = this.nbRetries;
        clone.productDate = this.productDate;
        clone.orbitId = this.orbitId;
        clone.statusReason = this.statusReason;
        if (this.tiles != null) {
            clone.tiles = new String[this.tiles.length];
            System.arraycopy(this.tiles, 0, clone.tiles, 0, clone.tiles.length);
        }
        clone.footprint = this.footprint;
        clone.orbitType = this.orbitType;
        return clone;
    }
}
