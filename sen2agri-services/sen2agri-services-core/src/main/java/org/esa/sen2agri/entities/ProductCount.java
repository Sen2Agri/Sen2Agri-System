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

import java.time.LocalDate;

public class ProductCount {
    private short siteId;
    private Satellite satellite;
    private LocalDate startDate;
    private LocalDate endDate;
    private int count;

    public short getSiteId() {
        return siteId;
    }
    public void setSiteId(short siteId) {
        this.siteId = siteId;
    }

    public Satellite getSatellite() {
        return satellite;
    }
    public void setSatellite(Satellite satellite) {
        this.satellite = satellite;
    }

    public LocalDate getStartDate() { return startDate; }
    public void setStartDate(LocalDate startDate) { this.startDate = startDate; }

    public LocalDate getEndDate() { return endDate; }
    public void setEndDate(LocalDate endDate) { this.endDate = endDate; }

    public int getCount() {
        return count;
    }
    public void setCount(int count) {
        this.count = count;
    }
}
