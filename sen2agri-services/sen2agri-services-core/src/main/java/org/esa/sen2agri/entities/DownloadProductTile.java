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
import org.esa.sen2agri.entities.enums.TileProcessingStatus;

import java.time.LocalDateTime;

public class DownloadProductTile {
    private Satellite satellite;
    private int orbitId;
    private String tileId;
    private TileProcessingStatus status;
    private int downloadProductId;
    private LocalDateTime statusTimestamp;
    private int retryCount;
    private String failReason;
    private Integer cloudCoverage;
    private Integer snowCoverage;

    public Satellite getSatellite() { return satellite; }
    public void setSatellite(Satellite satellite) { this.satellite = satellite; }

    public int getOrbitId() { return orbitId; }
    public void setOrbitId(int orbitId) { this.orbitId = orbitId; }

    public String getTileId() { return tileId; }
    public void setTileId(String tileId) { this.tileId = tileId; }

    public TileProcessingStatus getStatus() { return status; }
    public void setStatus(TileProcessingStatus status) { this.status = status; }

    public int getDownloadProductId() { return downloadProductId; }
    public void setDownloadProductId(int downloadProductId) { this.downloadProductId = downloadProductId; }

    public LocalDateTime getStatusTimestamp() { return statusTimestamp; }
    public void setStatusTimestamp(LocalDateTime statusTimestamp) { this.statusTimestamp = statusTimestamp; }

    public int getRetryCount() { return retryCount; }
    public void setRetryCount(int retryCount) { this.retryCount = retryCount; }

    public String getFailReason() { return failReason; }
    public void setFailReason(String failReason) { this.failReason = failReason; }

    public Integer getCloudCoverage() { return cloudCoverage; }
    public void setCloudCoverage(Integer cloudCoverage) { this.cloudCoverage = cloudCoverage; }

    public Integer getSnowCoverage() { return snowCoverage; }
    public void setSnowCoverage(Integer snowCoverage) { this.snowCoverage = snowCoverage; }

    public DownloadProductTile duplicate() {
        DownloadProductTile clone = new DownloadProductTile();
        clone.downloadProductId = this.downloadProductId;
        clone.satellite = this.satellite;
        clone.tileId = this.tileId;
        clone.orbitId = this.orbitId;
        clone.status = this.status;
        clone.statusTimestamp = this.statusTimestamp;
        clone.retryCount = this.retryCount;
        clone.failReason = this.failReason;
        clone.cloudCoverage = this.cloudCoverage;
        clone.snowCoverage = this.snowCoverage;
        return clone;
    }
}
