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
package org.esa.sen2agri.web;

import org.esa.sen2agri.services.ScheduleManager;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import ro.cs.tao.services.commons.ControllerBase;

import java.lang.management.ManagementFactory;
import java.lang.management.MemoryUsage;
import java.lang.management.ThreadInfo;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * @author Cosmin Udroiu
 */
@Controller
@RequestMapping("/refresh")
public class RefreshController extends ControllerBase {
    @Autowired
    private ScheduleManager scheduleManager;

    @RequestMapping(value = "/", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> refresh() {
        info("Refreshing configuration ...");
        scheduleManager.refresh();
        return new ResponseEntity<>("{}", HttpStatus.OK);
    }

    @RequestMapping(value = "/info", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> info() {
        MemoryUsage heapMemoryUsage = ManagementFactory.getMemoryMXBean().getHeapMemoryUsage();
        MemoryUsage nonHeapMemoryUsage = ManagementFactory.getMemoryMXBean().getNonHeapMemoryUsage();
        int threadCount = ManagementFactory.getThreadMXBean().getThreadCount();
        ThreadInfo[] threadInfos = ManagementFactory.getThreadMXBean().dumpAllThreads(true, true);
        Map<String, Object> info = new LinkedHashMap<>();
        info.put("Heap Memory Usage", heapMemoryUsage.toString());
        info.put("Non-heap Memory Usage", nonHeapMemoryUsage.toString());
        info.put("Thread Count", String.valueOf(threadCount));
        String[] threadNames = new String[threadInfos.length];
        for (int i = 0; i < threadNames.length; i++) {
            threadNames[i] = threadInfos[i].getThreadName();
        }
        info.put("Threads", threadNames);
        info.put("Quartz Jobs and Triggers", scheduleManager.getExecutingJobsWithTriggers());
        return new ResponseEntity<>(info, HttpStatus.OK);
    }
}
