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

import org.esa.sen2agri.entities.Season;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.ProductType;
import org.esa.sen2agri.services.DownloadService;
import org.esa.sen2agri.services.SiteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import ro.cs.tao.EnumUtils;
import ro.cs.tao.services.commons.ControllerBase;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Controller
@RequestMapping("/sites")
public class SiteController extends ControllerBase {

    @Autowired
    private SiteService siteService;
    @Autowired
    private DownloadService downloadService;

    @RequestMapping(value = "/", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<SiteInfoPayload>> list() {
        List<Site> objects = siteService.list();
        if (objects == null || objects.isEmpty()) {
            return new ResponseEntity<>(new ArrayList<>(), HttpStatus.OK);
        }
        List<SiteInfoPayload> payload = objects.stream()
                .map(p -> new SiteInfoPayload(p.getId(), p.getName(), p.getShortName(), p.isEnabled()))
                .collect(Collectors.toList());

        return new ResponseEntity<>(payload, HttpStatus.OK);
    }

    @RequestMapping(value = "/", method = RequestMethod.POST, produces = "application/json")
    public ResponseEntity<?> create(@RequestBody CreateSitePayload entity) {
        int siteId = siteService.createSite(entity.getName(), entity.getZipFilePath(), entity.isEnabled());
        return new ResponseEntity<>(siteId, HttpStatus.OK);
    }

    @RequestMapping(value = "/", method = RequestMethod.DELETE, produces = "application/json")
    public ResponseEntity<?> delete(@RequestBody DeleteReqPayload entity) {
        short siteId = entity.siteId;
        if (entity.siteId < 0) {
            siteId = siteService.getSiteId(entity.siteShortName);
        }
        // First stop also the downloads
        downloadService.stop(siteId, (short)1);
        downloadService.stop(siteId, (short)2);
        siteService.deleteSite(siteId, entity.productTypeIds != null ?
                entity.productTypeIds.stream().map(pt -> EnumUtils.getEnumConstantByValue(ProductType.class, pt)).toArray(ProductType[]::new) :
                null);
        return new ResponseEntity<>("{}", HttpStatus.OK);
    }


    @RequestMapping(value = "/seasons/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<Season>> getSiteSeasons(@PathVariable("id") String id) {
        List<Season> seasons = siteService.getSiteSeason(id);
        if (seasons == null || seasons.isEmpty()) {
            return new ResponseEntity<>(new ArrayList<>(), HttpStatus.OK);
        }
        return new ResponseEntity<>(seasons, HttpStatus.OK);
    }

    private static class DeleteReqPayload
    {
        private short siteId;
        private String siteShortName;
        private List<Integer> productTypeIds;

        public DeleteReqPayload()
        {
        }

        public short getSiteId() {
            return siteId;
        }

        public void setSiteId(short siteId) {
            this.siteId = siteId;
        }

        public List<Integer> getProductTypeIds() {
            return productTypeIds;
        }

        public void setProductTypeIds(List<Integer> productTypeIds) {
            this.productTypeIds = productTypeIds;
        }

        public String getSiteShortName() {
            return siteShortName;
        }

        public void setSiteShortName(String siteShortName) {
            this.siteShortName = siteShortName;
        }
    }

    private static class SiteInfoPayload
    {
        private short id;
        private String name;
        private String shortName;
        //private String extent;
        private boolean enabled;
        //private List<SiteTiles> tiles;

        public SiteInfoPayload() {
        }

        public SiteInfoPayload(short id, String name, String shortName, boolean enabled) {
            this.id = id;
            this.name = name;
            this.shortName = shortName;
            this.enabled = enabled;
        }

        public short getId() {
            return id;
        }

        public void setId(short id) {
            this.id = id;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getShortName() {
            return shortName;
        }

        public void setShortName(String shortName) {
            this.shortName = shortName;
        }

        public boolean isEnabled() {
            return enabled;
        }

        public void setEnabled(boolean enabled) {
            this.enabled = enabled;
        }
    }

    private static class CreateSitePayload
    {
        private String name;
        private String zipFilePath;
        private boolean enabled;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getZipFilePath() {
            return zipFilePath;
        }

        public void setZipFilePath(String zipFilePath) {
            this.zipFilePath = zipFilePath;
        }

        public boolean isEnabled() {
            return enabled;
        }

        public void setEnabled(boolean enabled) {
            this.enabled = enabled;
        }
    }

}
