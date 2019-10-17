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
import org.esa.sen2agri.entities.enums.Status;

public class DownloadSummary {
    private String siteName;
    private int year;
    private Satellite satellite;
    private Status status;
    private long count;

    public String getSiteName() { return siteName; }

    public void setSiteName(String siteName) { this.siteName = siteName; }

    public int getYear() { return year; }

    public void setYear(int year) {
        this.year = year;
    }

    public Satellite getSatellite() { return satellite; }

    public void setSatellite(Satellite satellite) { this.satellite = satellite; }

    public Status getStatus() {
        return status;
    }

    public void setStatus(Status status) {
        this.status = status;
    }

    public long getCount() { return count; }

    public void setCount(long count) { this.count = count; }
}
