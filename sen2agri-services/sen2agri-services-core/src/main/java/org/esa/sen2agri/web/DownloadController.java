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

import org.esa.sen2agri.commons.TaskProgress;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.services.DownloadService;
import org.esa.sen2agri.services.ScheduleManager;
import org.esa.sen2agri.services.SensorProgress;
import org.esa.sen2agri.services.SiteHelper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import ro.cs.tao.services.commons.ControllerBase;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Cosmin Cara
 */
@Controller
@RequestMapping("/downloader")
public class DownloadController extends ControllerBase {

    @Autowired
    private DownloadService downloadService;
    @Autowired
    private SiteHelper siteHelper;
    @Autowired
    private ScheduleManager scheduleManager;

    /**
     * Returns information about all the downloads in progress.
     */
    @RequestMapping(value = "/", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<TaskProgress>> getInProgress() {
        List<TaskProgress> tasks = downloadService.getDownloadsInProgress((short) 0);
        if (tasks == null || tasks.isEmpty()) {
            return new ResponseEntity<>(HttpStatus.OK);
        }
        return new ResponseEntity<>(tasks, HttpStatus.OK);
    }

    /**
     * Returns information about the downloads in progress for a specific site
     * @param siteId    The site identifier
     */
    @RequestMapping(value = "/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<TaskProgress>> getInProgress(@PathVariable("id") short siteId) {
        List<TaskProgress> tasks = downloadService.getDownloadsInProgress(siteId);
        if (tasks == null || tasks.isEmpty()) {
            return new ResponseEntity<>(HttpStatus.OK);
        }
        return new ResponseEntity<>(tasks, HttpStatus.OK);
    }

    @RequestMapping(value = "/{code}/{satellite}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<SensorProgress>> getOverallProgress(@PathVariable("code") String shortName,
                                                                 @PathVariable("satellite") String satelliteName) {
        Short siteId = siteHelper.getSiteIdByShortName(shortName);
        Satellite satellite = Enum.valueOf(Satellite.class, satelliteName);
        List<SensorProgress> progress = null;
        if (siteId != null) {
            progress = downloadService.getProgress(siteId, satellite.value());
            if (progress == null) {
                progress = new ArrayList<>();
            }
        }
        return new ResponseEntity<>(progress, HttpStatus.OK);
    }

    @RequestMapping(value = "/{id}/count", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<Long> getCount(@PathVariable("id") short siteId) {
        long count = downloadService.getCount(siteId);
        return new ResponseEntity<>(count, HttpStatus.OK);
    }

    /**
     * Stops all the downloads and marks the downloader as disabled for all sites
     */
    @RequestMapping(value = "/stop", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> stop() {
        downloadService.stop((short) 0);
        info("/downloader/stop received");
        return new ResponseEntity<>("Stop message sent", HttpStatus.OK);
    }

    /**
     * Stops the downloads and marks the downloader disabled for the specific site.
     * @param siteId    The site identifier
     */
    @RequestMapping(value = "/stop/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> stop(@PathVariable("id") short siteId) {
        downloadService.stop(siteId);
        info("/downloader/stop/%s received", siteId);
        return new ResponseEntity<>("Stop message sent", HttpStatus.OK);
    }

    /**
     * Stops the downloads of the specific satellite and marks the downloader disabled
     * only for the specific site and satellite
     * @param siteId    The site identifier
     * @param satelliteId   The satellite identifier
     */
    @RequestMapping(value = "/stop/{id}/{satelliteId}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> stop(@PathVariable("id") short siteId,
                                  @PathVariable("satelliteId") short satelliteId) {
        downloadService.stop(siteId, satelliteId);
        info("/downloader/stop/%s/%s received", siteId, satelliteId);
        return new ResponseEntity<>("Stop message sent", HttpStatus.OK);
    }

    /**
     * Enables the downloader.
     */
    @RequestMapping(value = "/start", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> start() {
        downloadService.start((short) 0);
        scheduleManager.refresh();
        info("/downloader/start received");
        return new ResponseEntity<>("Start message sent", HttpStatus.OK);
    }

    /**
     * Enables the downloader for the specific site.
     * @param siteId    The site identifier
     */
    @RequestMapping(value = "/start/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> start(@PathVariable("id") short siteId) {
        downloadService.start(siteId);
        scheduleManager.refresh();
        info("/downloader/start/%s received", siteId);
        return new ResponseEntity<>("Start message sent", HttpStatus.OK);
    }

    /**
     * Forces the downloader to start from the beginning for the specific site.
     * @param siteId    The site identifier
     */
    @RequestMapping(value = "/forcestart/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> forceStart(@PathVariable("id") short siteId) {
        downloadService.forceStart(siteId);
        scheduleManager.refresh();
        info("/downloader/forcestart/%s received", siteId);
        return new ResponseEntity<>("Force start message sent", HttpStatus.OK);
    }

    /**
     * Enables the downloader for the specific site and satellite.
     * @param siteId    The site identifier
     * @param satelliteId   The satellite identifier
     */
    @RequestMapping(value = "/start/{id}/{satelliteId}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> start(@PathVariable("id") short siteId,
                                   @PathVariable("satelliteId") short satelliteId) {
        downloadService.start(siteId, satelliteId);
        scheduleManager.refresh();
        info("/downloader/start/%s/%s received", siteId, satelliteId);
        return new ResponseEntity<>("Start message sent", HttpStatus.OK);
    }

    /**
     * Forces the downloader to start from the beginning for the specific site and satellite.
     * @param siteId    The site identifier
     * @param satelliteId   The satellite identifier
     */
    @RequestMapping(value = "/forcestart/{id}/{satelliteId}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> forceStart(@PathVariable("id") short siteId,
                                        @PathVariable("satelliteId") short satelliteId) {
        downloadService.forceStart(siteId, satelliteId);
        scheduleManager.refresh();
        info("/downloader/forcestart/%s/%s received", siteId, satelliteId);
        return new ResponseEntity<>("Force message sent", HttpStatus.OK);
    }
}
