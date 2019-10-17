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

import org.esa.sen2agri.entities.enums.ActivityStatus;
import org.esa.sen2agri.entities.enums.JobStartType;

import java.time.LocalDateTime;

public class Job {
    private int id;
    private Processor processor;
    private Site site;
    private JobStartType jobStartType;
    private String parameters;
    private LocalDateTime submitTimestamp;
    private LocalDateTime startTimestamp;
    private LocalDateTime endTimestamp;
    private ActivityStatus status;
    private LocalDateTime statusTimestamp;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public Processor getProcessor() {
        return processor;
    }

    public void setProcessor(Processor processor) {
        this.processor = processor;
    }

    public Site getSite() {
        return site;
    }

    public void setSite(Site site) {
        this.site = site;
    }

    public JobStartType getJobStartType() {
        return jobStartType;
    }

    public void setJobStartType(JobStartType jobStartType) {
        this.jobStartType = jobStartType;
    }

    public String getParameters() {
        return parameters;
    }

    public void setParameters(String parameters) {
        this.parameters = parameters;
    }

    public LocalDateTime getSubmitTimestamp() {
        return submitTimestamp;
    }

    public void setSubmitTimestamp(LocalDateTime submitTimestamp) {
        this.submitTimestamp = submitTimestamp;
    }

    public LocalDateTime getStartTimestamp() {
        return startTimestamp;
    }

    public void setStartTimestamp(LocalDateTime startTimestamp) {
        this.startTimestamp = startTimestamp;
    }

    public LocalDateTime getEndTimestamp() {
        return endTimestamp;
    }

    public void setEndTimestamp(LocalDateTime endTimestamp) {
        this.endTimestamp = endTimestamp;
    }

    public ActivityStatus getStatus() {
        return status;
    }

    public void setStatus(ActivityStatus status) {
        this.status = status;
    }

    public LocalDateTime getStatusTimestamp() {
        return statusTimestamp;
    }

    public void setStatusTimestamp(LocalDateTime statusTimestamp) {
        this.statusTimestamp = statusTimestamp;
    }
}
