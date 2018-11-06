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
package org.esa.sen2agri;

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.services.ScheduleManager;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ImportResource;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.SchedulingConfigurer;
import org.springframework.scheduling.config.IntervalTask;
import org.springframework.scheduling.config.ScheduledTaskRegistrar;
import ro.cs.tao.services.commons.ServiceLauncher;
import ro.cs.tao.datasource.util.NetUtils;

import java.util.logging.Logger;

/**
 * @author Cosmin Cara
 */
@SpringBootApplication
@EnableScheduling
@ImportResource("classpath:downloader-service-context.xml")
public class CoreLauncher implements SchedulingConfigurer, ServiceLauncher {

    @Autowired
    private ScheduleManager scheduleManager;

    public static void main(String[] args) {
        SpringApplication.run(CoreLauncher.class, args);
    }

    @Override
    public void configureTasks(ScheduledTaskRegistrar taskRegistrar) {
        long interval = Long.parseLong(Config.getProperty("database.config.polling", "15")) * 60000;
        int netUtilTimeout = Integer.parseInt(Config.getProperty("network.connexions.timeout", "30")) * 1000;
        NetUtils.setTimeout(netUtilTimeout);
        Logger.getLogger(CoreLauncher.class.getName())
                .info(String.format("Network connection timeout initialized at %d seconds", netUtilTimeout / 1000));

        taskRegistrar.addFixedRateTask(new IntervalTask(() -> {
            scheduleManager.refresh();
        }, interval, interval));
        Logger.getLogger(CoreLauncher.class.getName())
                .info(String.format("Database configuration polling initialized at %d minutes", interval / 60000));
    }
}