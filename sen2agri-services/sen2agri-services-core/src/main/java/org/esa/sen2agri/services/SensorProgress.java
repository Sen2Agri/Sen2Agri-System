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

package org.esa.sen2agri.services;

import java.time.LocalDate;

public class SensorProgress {
    private LocalDate begin;
    private LocalDate end;
    private int estimated;
    private int downloaded;

    public LocalDate getBegin() { return begin; }
    public void setBegin(LocalDate begin) { this.begin = begin; }

    public LocalDate getEnd() { return end; }
    public void setEnd(LocalDate end) { this.end = end; }

    public int getEstimated() {
        return estimated;
    }
    public void setEstimated(int estimated) {
        this.estimated = estimated;
    }

    public int getDownloaded() {
        return downloaded;
    }
    public void setDownloaded(int downloaded) {
        this.downloaded = downloaded;
    }
}
