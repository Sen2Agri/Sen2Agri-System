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

import java.time.LocalDateTime;

/**
 * @author Cosmin Cara
 */
public class ScheduledTaskStatus {
    private int id;
    private ScheduledTask task;
    private LocalDateTime nextSchedule;
    private LocalDateTime lastScheduledRun;
    private LocalDateTime lastRun;
    private LocalDateTime lastRetry;
    private LocalDateTime estimatedNextRun;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public ScheduledTask getTask() {
        return task;
    }

    public void setTask(ScheduledTask task) {
        this.task = task;
    }

    public LocalDateTime getNextSchedule() {
        return nextSchedule;
    }

    public void setNextSchedule(LocalDateTime nextSchedule) {
        this.nextSchedule = nextSchedule;
    }

    public LocalDateTime getLastScheduledRun() {
        return lastScheduledRun;
    }

    public void setLastScheduledRun(LocalDateTime lastScheduledRun) {
        this.lastScheduledRun = lastScheduledRun;
    }

    public LocalDateTime getLastRun() {
        return lastRun;
    }

    public void setLastRun(LocalDateTime lastRun) {
        this.lastRun = lastRun;
    }

    public LocalDateTime getLastRetry() {
        return lastRetry;
    }

    public void setLastRetry(LocalDateTime lastRetry) {
        this.lastRetry = lastRetry;
    }

    public LocalDateTime getEstimatedNextRun() {
        return estimatedNextRun;
    }

    public void setEstimatedNextRun(LocalDateTime estimatedNextRun) {
        this.estimatedNextRun = estimatedNextRun;
    }
}
