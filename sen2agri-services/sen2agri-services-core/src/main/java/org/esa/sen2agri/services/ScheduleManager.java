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

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.scheduling.Job;
import org.esa.sen2agri.scheduling.JobDescriptor;
import org.quartz.JobDetail;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.Trigger;
import org.quartz.impl.StdSchedulerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import ro.cs.tao.eodata.Polygon2D;
import ro.cs.tao.spi.ServiceRegistryManager;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.logging.Logger;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Component("scheduleManager")
public class ScheduleManager {

    private Logger logger;

    @Autowired
    private DownloadService service;
    @Autowired
    private PersistenceManager persistenceManager;

    private Scheduler scheduler;

    @PostConstruct
    public void initialize() throws SchedulerException {
        logger = Logger.getLogger(ScheduleManager.class.getName());
        Config.setPersistenceManager(persistenceManager);
        Config.setDownloadService(service);
        service.setProductStatusListener(new ProductDownloadListener(persistenceManager));
        scheduler = StdSchedulerFactory.getDefaultScheduler();
        scheduler.start();
        List<Site> enabledSites = getEnabledSites();
        logger.info("Enabled sites: " +
                            String.join(",",
                                        enabledSites.stream().map(Site::getShortName).collect(Collectors.toList())));
        Set<Job> jobs = ServiceRegistryManager.getInstance().getServiceRegistry(Job.class).getServices();
        logger.info("Found scheduled job types: " +
                            String.join(",",
                                        jobs.stream().map(Job::groupName).collect(Collectors.toList())));
        for (Job job : jobs) {
            schedule(job, enabledSites);
        }
    }

    @PreDestroy
    public void shutdown() throws SchedulerException {
        scheduler.shutdown();
    }

    public void refresh() {
        Config.reload();
        logger.fine("Configuration refreshed from database ");
        List<Site> enabledSites = getEnabledSites();
        Set<Job> jobs = ServiceRegistryManager.getInstance().getServiceRegistry(Job.class).getServices();
        for (Job job : jobs) {
            schedule(job, enabledSites);
        }
    }

    private List<Site> getEnabledSites() {
        final List<Site> enabledSites = persistenceManager.getEnabledSites();
        enabledSites.removeIf(s -> Polygon2D.fromWKT(s.getExtent()) == null);
        return enabledSites;
    }

    private void schedule(Job job, List<Site> sites) {
        final Map<Satellite, List<DataSourceConfiguration>> queryConfigurations = Config.getQueryConfigurations();
        final Map<Satellite, List<DataSourceConfiguration>> downloadConfigurations = Config.getDownloadConfigurations();
        downloadConfigurations.keySet().forEach(s -> {
            final Set<Satellite> filter = job.getSatelliteFilter();
            if (filter != null && !filter.contains(s)) {
                logger.fine(String.format("Job '%s' is not intended for sensor '%s'",
                                          job.configKey(), s.shortName()));
                return;
            }
            if (Config.isFeatureEnabled((short) 0, job.configKey())) {
                String sensorCode = s.shortName().toLowerCase();
                if (Config.isFeatureEnabled((short) 0, String.format(ConfigurationKeys.SENSOR_STATE, sensorCode)) ||
                        Config.isFeatureEnabled((short) 0, String.format(ConfigurationKeys.DOWNLOADER_SITE_STATE_ENABLED, sensorCode))) {
                    for (Site site : sites) {
                        JobDetail jobDetail = null;
                        try {
                            if (!queryConfigurations.containsKey(s)) {
                                throw new SchedulerException(String.format("No query configuration found for satellite %s",
                                                                           s.shortName()));
                            }
                            final DataSourceConfiguration queryConfig = queryConfigurations.get(s).stream()
                                    .filter(ds -> (ds.getSiteId() != null && ds.getSiteId().equals(site.getId())) ||
                                                  (ds.getSiteId() == null))
                                    .findFirst().orElse(null);
                            if (queryConfig == null) {
                                throw new SchedulerException(String.format("Cannot find query configuration for site %s",
                                                                           site.getShortName()));
                            }
                            if (!downloadConfigurations.containsKey(s)) {
                                throw new SchedulerException(String.format("No download configuration found for satellite %s",
                                                                           s.shortName()));
                            }
                            final DataSourceConfiguration downloadConfig = downloadConfigurations.get(s).stream()
                                    .filter(ds -> (ds.getSiteId() != null && ds.getSiteId().equals(site.getId())) ||
                                                  (ds.getSiteId() == null))
                                    .findFirst().orElse(null);
                            if (downloadConfig == null) {
                                throw new SchedulerException(String.format("Cannot find download configuration for site %s",
                                                                           site.getShortName()));
                            }
                            final JobDescriptor descriptor = job.createDescriptor(site.getShortName() + "-" + s.name(), downloadConfig.getRetryInterval());

                            jobDetail = job.buildDetail(site, s, queryConfig, downloadConfig);
                            final Trigger trigger = descriptor.buildTrigger();

                            if (scheduler.checkExists(jobDetail.getKey())) {
                                JobDetail previousJobDetail = scheduler.getJobDetail(jobDetail.getKey());
                                final Site oldSite = (Site) previousJobDetail.getJobDataMap().get("site");
                                if (!site.equals(oldSite)) {
                                    try {
                                        scheduler.deleteJob(previousJobDetail.getKey());
                                    } catch (Exception e1) {
                                        logger.warning(String.format("Unable to delete job '%s' (next run: %s)",
                                                                     jobDetail.getKey(), descriptor.getFireTime()));
                                    }
                                    scheduler.scheduleJob(jobDetail, trigger);
                                    logger.info(String.format("Scheduled job '%s' (site info changed, next run: %s)",
                                                              jobDetail.getKey(), descriptor.getFireTime()));
                                } else {
                                    scheduler.rescheduleJob(trigger.getKey(), trigger);
                                    logger.info(String.format("Rescheduled job '%s' (next run: %s)",
                                                              jobDetail.getKey(), descriptor.getFireTime()));
                                }
                            } else {
                                scheduler.scheduleJob(jobDetail, trigger);
                                logger.info(String.format("Scheduled job '%s' (next run: %s)",
                                                          jobDetail.getKey(), descriptor.getFireTime()));
                            }
                        } catch (SchedulerException e) {
                            if (jobDetail != null) {
                                logger.severe(String.format("Failed to schedule job '%s'. Reason: %s",
                                                            jobDetail.getKey(),
                                                            e.getMessage()));
                            } else {
                                logger.severe(String.format("Failed to create job for site '%s' and satellite '%s'. Reason: %s",
                                                            site.getShortName(), s.shortName(),
                                                            e.getMessage()));
                            }
                        }
                    }
                } else {
                    logger.info(String.format("Sensor '%s' is globally disabled", s.name()));
                }
            } else {
                logger.info(String.format("Setting %s is disabled", job.configKey()));
            }
        });
    }
}
