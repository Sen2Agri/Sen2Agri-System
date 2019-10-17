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
package org.esa.sen2agri.scheduling;

import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.Satellite;
import org.quartz.JobDetail;

import java.time.LocalDateTime;
import java.util.Set;

/**
 * @author Cosmin Cara
 */
public interface Job extends org.quartz.Job {
    String configKey();
    String groupName();
    String getId();
    void setId(String id);
    default boolean isSingleInstance() { return false; }
    /**
     * Returns the set of satellites for which this job is intended.
     * By default, jobs are intended for all satellites, therefore, override the method in a subclass that is
     * intended only for a subset of satellites.
     */
    default Set<Satellite> getSatelliteFilter() { return null; }
    default JobDescriptor createDescriptor(int rateInMinutes) {
        return new JobDescriptor()
                .setName(getId())
                .setGroup(groupName())
                .setFireTime(LocalDateTime.now().plusSeconds(10))
                .setRate(rateInMinutes);
    }
    default JobDetail buildDetail(Site site,
                                  DataSourceConfiguration queryConfig,
                                  DataSourceConfiguration downloadConfig) {
        return createDescriptor(downloadConfig.getRetryInterval())
                .buildJobDetail(getClass(), site, queryConfig, downloadConfig);
    }
}
