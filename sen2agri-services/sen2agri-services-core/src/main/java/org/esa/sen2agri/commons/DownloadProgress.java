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

import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.Satellite;
import ro.cs.tao.datasource.DataSourceTopic;
import ro.cs.tao.messaging.TaskProgress;

import java.util.Objects;

/**
 * @author Cosmin Cara
 */
public class DownloadProgress extends TaskProgress {
    private String siteName;
    private int siteId;
    private String satelliteName;

    public DownloadProgress(String name) {
        super(name, DataSourceTopic.PRODUCT_PROGRESS.getTag());
    }

    public DownloadProgress(String name, Site site, Satellite satellite, double progress) {
        this(name);
        if (site != null) {
            this.siteName = site.getName();
            this.siteId = site.getId();
        }
        if (satellite != null) {
            this.satelliteName = satellite.name();
        }
        this.progress = progress;
    }

    public String getSiteName() { return siteName; }

    public int getSiteId() { return siteId; }

    public String getSatelliteName() { return satelliteName; }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        if (!super.equals(o)) return false;
        DownloadProgress that = (DownloadProgress) o;
        return siteId == that.siteId &&
                siteName.equals(that.siteName) &&
                satelliteName.equals(that.satelliteName);
    }

    @Override
    public int hashCode() {
        return Objects.hash(super.hashCode(), siteName, siteId, satelliteName);
    }
}
