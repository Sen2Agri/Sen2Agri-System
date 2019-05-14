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

import java.time.LocalDate;
import java.time.format.DateTimeFormatter;

/**
 * @author Cosmin Cara
 */
public class Season {
    private short id;
    private short siteId;
    private String name;
    private LocalDate startDate;
    private LocalDate endDate;
    private LocalDate midDate;
    private boolean enabled;

    public short getId() {
        return id;
    }

    public void setId(short id) {
        this.id = id;
    }

    public short getSiteId() {
        return siteId;
    }

    public void setSiteId(short siteId) {
        this.siteId = siteId;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public LocalDate getStartDate() {
        return startDate;
    }

    public void setStartDate(LocalDate startDate) {
        this.startDate = startDate;
    }

    public LocalDate getEndDate() {
        return endDate;
    }

    public void setEndDate(LocalDate endDate) {
        this.endDate = endDate;
    }

    public LocalDate getMidDate() {
        return midDate;
    }

    public void setMidDate(LocalDate midDate) {
        this.midDate = midDate;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    @Override
    public String toString() {
        return name + " [" + startDate.format(DateTimeFormatter.ofPattern("yyyy-MM-dd")) + "," +
                midDate.format(DateTimeFormatter.ofPattern("yyyy-MM-dd")) + "," +
                endDate.format(DateTimeFormatter.ofPattern("yyyy-MM-dd")) + "]";
    }
}
