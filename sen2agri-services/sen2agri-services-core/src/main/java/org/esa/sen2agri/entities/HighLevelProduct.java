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

import org.locationtech.jts.geom.Geometry;

import java.time.LocalDateTime;

/**
 * @author Cosmin Cara
 */
public class HighLevelProduct {
    private int id;
    private ProductType productType;
    private int processorId;
    private short siteId;
    private String fullPath;
    private LocalDateTime created;
    private LocalDateTime inserted;
    private boolean archived;
    private LocalDateTime archivedTimestamp;
    private String productName;
    private Satellite satellite;
    private Geometry footprint;
    private int relativeOrbit;
    private String[] tiles;
    private OrbitType orbitType;
    private int downloadProductId;

    public int getId() {
        return id;
    }
    public void setId(int id) {
        this.id = id;
    }

    public ProductType getProductType() {
        return productType;
    }
    public void setProductType(ProductType productType) {
        this.productType = productType;
    }

    public int getProcessorId() {
        return processorId;
    }
    public void setProcessorId(int processorId) {
        this.processorId = processorId;
    }

    public short getSiteId() {
        return siteId;
    }
    public void setSiteId(short value) {
        this.siteId = value;
    }

    public String getFullPath() {
        return fullPath;
    }
    public void setFullPath(String fullPath) {
        this.fullPath = fullPath;
    }

    public LocalDateTime getCreated() {
        return created;
    }
    public void setCreated(LocalDateTime created) {
        this.created = created;
    }

    public LocalDateTime getInserted() {
        return inserted;
    }
    public void setInserted(LocalDateTime inserted) {
        this.inserted = inserted;
    }

    public boolean isArchived() {
        return archived;
    }
    public void setArchived(boolean archived) {
        this.archived = archived;
    }

    public LocalDateTime getArchivedTimestamp() {
        return archivedTimestamp;
    }
    public void setArchivedTimestamp(LocalDateTime archivedTimestamp) {
        this.archivedTimestamp = archivedTimestamp;
    }

    public String getProductName() {
        return productName;
    }
    public void setProductName(String name) {
        this.productName = name;
    }

    public Satellite getSatellite() {
        return satellite;
    }
    public void setSatellite(Satellite satellite) {
        this.satellite = satellite;
    }

    public Geometry getFootprint() { return footprint; }
    public void setFootprint(Geometry footprint) { this.footprint = footprint; }

    public int getRelativeOrbit() { return relativeOrbit; }
    public void setRelativeOrbit(int relativeOrbit) { this.relativeOrbit = relativeOrbit; }

    public String[] getTiles() {
        return tiles;
    }
    public void setTiles(String[] tiles) {
        this.tiles = tiles;
    }

    public OrbitType getOrbitType() { return orbitType; }
    public void setOrbitType(OrbitType orbitType) { this.orbitType = orbitType; }

    public int getDownloadProductId() { return downloadProductId; }
    public void setDownloadProductId(int downloadProductId) { this.downloadProductId = downloadProductId; }
}
