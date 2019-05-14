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
public class ScheduledTask {
    private int id;
    private String name;
    private Processor processor;
    private Site site;
    private Season season;
    private String parameters;
    private int repeatType;
    private int repeatAfterDays;
    private int repeatOnMonthDay;
    private int retrySeconds;
    private int priority;
    private LocalDateTime firstRun;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
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

    public Season getSeason() {
        return season;
    }

    public void setSeason(Season season) {
        this.season = season;
    }

    public String getParameters() {
        return parameters;
    }

    public void setParameters(String parameters) {
        this.parameters = parameters;
    }

    public int getRepeatType() {
        return repeatType;
    }

    public void setRepeatType(int repeatType) {
        this.repeatType = repeatType;
    }

    public int getRepeatAfterDays() {
        return repeatAfterDays;
    }

    public void setRepeatAfterDays(int repeatAfterDays) {
        this.repeatAfterDays = repeatAfterDays;
    }

    public int getRepeatOnMonthDay() {
        return repeatOnMonthDay;
    }

    public void setRepeatOnMonthDay(int repeatOnMonthDay) {
        this.repeatOnMonthDay = repeatOnMonthDay;
    }

    public int getRetrySeconds() {
        return retrySeconds;
    }

    public void setRetrySeconds(int retrySeconds) {
        this.retrySeconds = retrySeconds;
    }

    public int getPriority() {
        return priority;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public LocalDateTime getFirstRun() {
        return firstRun;
    }

    public void setFirstRun(LocalDateTime firstRun) {
        this.firstRun = firstRun;
    }
}
