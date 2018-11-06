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
import org.esa.sen2agri.entities.Site;

/**
 * @author Cosmin Cara
 */
public class TaskProgress {
    private String name;
    private String siteName;
    private int siteId;
    private String satelliteName;
    private double progress;
    //private SubTaskProgress subTaskProgress;

    public TaskProgress(String name) {
        this.name = name;
    }

    public TaskProgress(String name, Site site, Satellite satellite, double progress) {//, SubTaskProgress subTaskProgress) {
        this.name = name;
        if (site != null) {
            this.siteName = site.getName();
            this.siteId = site.getId();
        }
        if (satellite != null) {
            this.satelliteName = satellite.name();
        }
        this.progress = progress;
        //this.subTaskProgress = subTaskProgress;
    }

    /*public TaskProgress(String name, Site site, Satellite satellite, double progress, String subTask, double subProgress) {
        this(name, site, satellite, progress, new SubTaskProgress(subTask, subProgress));
    }*/

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public double getProgress() {
        return progress;
    }

    public void setProgress(double progress) {
        this.progress = progress;
    }

    /*public SubTaskProgress getSubTaskProgress() {
        return subTaskProgress;
    }

    public void setSubTaskProgress(SubTaskProgress subTaskProgress) {
        this.subTaskProgress = subTaskProgress;
    }*/

    public String getSiteName() { return siteName; }
    public void setSiteName(String siteName) { this.siteName = siteName; }

    public int getSiteId() { return siteId; }
    public void setSiteId(int siteId) { this.siteId = siteId; }

    public String getSatelliteName() { return satelliteName; }
    public void setSatelliteName(String satelliteName) { this.satelliteName = satelliteName; }

    @Override
    public int hashCode() {
        return this.name.hashCode();
    }
}
