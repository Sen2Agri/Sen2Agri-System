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
import org.esa.sen2agri.commons.Constants;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.Parameter;
import org.esa.sen2agri.entities.Season;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.web.beans.Query;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import ro.cs.tao.datasource.param.CommonParameterNames;
import ro.cs.tao.eodata.Polygon2D;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.logging.Logger;

@Component("siteHelper")
public class SiteHelper {

    @Autowired
    private PersistenceManager persistenceManager;

    private Logger logger = Logger.getLogger(SiteHelper.class.getName());

    public void setPersistenceManager(PersistenceManager persistenceManager) {
        this.persistenceManager = persistenceManager;
    }

    public Short getSiteIdByShortName(String shortName) {
        Site site = persistenceManager.getSiteByShortName(shortName);
        return site != null ? site.getId() : -1;
    }

    public LocalDateTime getStartDate(Site site, Satellite satellite) {
        final List<Season> seasons = persistenceManager.getEnabledSeasons(site.getId());
        LocalDateTime minStartDate = seasons.get(0).getStartDate().atStartOfDay();
        int downloaderStartOffset = Config.getAsInteger(ConfigurationKeys.DOWNLOADER_START_OFFSET, 0);
        if (downloaderStartOffset > 0) {
            minStartDate = minStartDate.minusMonths(downloaderStartOffset);
        }
        return minStartDate;
    }

    public LocalDateTime getEndDate(Site site, Satellite satellite) {
        final List<Season> seasons = persistenceManager.getEnabledSeasons(site.getId());
        LocalDateTime endDate = seasons.get(seasons.size() - 1).getEndDate().atStartOfDay().plusDays(1).minusSeconds(1);
        if (LocalDateTime.now().compareTo(endDate) < 0) {
            endDate = LocalDateTime.now();
        }
        return endDate;
    }

    public Query createQuery(Site site, DataSourceConfiguration queryConfiguration,
                             LocalDateTime start, LocalDateTime end) {
        Query query = new Query();
        query.setUser(queryConfiguration.getUser());
        query.setPassword(queryConfiguration.getPassword());
        Map<String, Object> params = new HashMap<>();
        Satellite satellite = queryConfiguration.getSatellite();
        Set<String> tiles = persistenceManager.getSiteTiles(site, satellite);
        //params.put(ParameterHelper.getStartDateParamName(satellite),
        params.put(CommonParameterNames.START_DATE,
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        //params.put(ParameterHelper.getEndDateParamName(satellite),
        params.put(CommonParameterNames.END_DATE,
                   new String[] {
                           start.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)),
                           end.format(DateTimeFormatter.ofPattern(Constants.FULL_DATE_FORMAT)) });
        //params.put(ParameterHelper.getFootprintParamName(satellite), Polygon2D.fromWKT(site.getExtent()));
        params.put(CommonParameterNames.FOOTPRINT, Polygon2D.fromWKT(site.getExtent()));
        //String tileParamName = ParameterHelper.getTileParamName(satellite);
        if (tiles != null && tiles.size() > 0) {
            String tileList;
            if (tiles.size() == 1) {
                tileList = tiles.stream().findFirst().get();
            } else {
                tileList = "[" + String.join(",", tiles) + "]";
            }
            params.put(CommonParameterNames.TILE, tileList);
        }
        final List<Parameter> specificParameters = queryConfiguration.getSpecificParameters();
        if (specificParameters != null) {
            specificParameters.forEach(p -> {
                try {
                    params.put(p.getName(), p.typedValue());
                } catch (Exception e) {
                    logger.warning(String.format("Incorrect typed value for parameter [%s] : expected '%s', found '%s'",
                                                 p.getName(), p.getType(), p.getValue()));
                }
            });
        }
        query.setValues(params);
        return query;
    }

    public List<LocalDateTime[]> getDownloadIntervals(short siteId) {
        List<LocalDateTime[]> dates = new ArrayList<>();
        final List<Season> seasons = persistenceManager.getEnabledSeasons(siteId);
        if (seasons != null && seasons.size() > 0) {
            seasons.sort(Comparator.comparing(Season::getStartDate));
            for (Season season : seasons) {
                LocalDateTime minStartDate = seasons.get(0).getStartDate().atStartOfDay();
                int downloaderStartOffset = Config.getAsInteger(ConfigurationKeys.DOWNLOADER_START_OFFSET, 1);
                if (downloaderStartOffset > 0) {
                    minStartDate = minStartDate.minusMonths(downloaderStartOffset);
                }
                LocalDateTime endDate = seasons.get(seasons.size() - 1).getEndDate().atStartOfDay().minusSeconds(1);
                if (LocalDateTime.now().compareTo(endDate) < 0) {
                    endDate = LocalDateTime.now();
                }
                dates.add(new LocalDateTime[] { minStartDate, endDate });
            }
        }
        return dates;
    }
}
