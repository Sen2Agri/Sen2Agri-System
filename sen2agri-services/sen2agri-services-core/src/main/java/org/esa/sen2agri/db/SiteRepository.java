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
package org.esa.sen2agri.db;

import org.esa.sen2agri.entities.Site;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.PagingAndSortingRepository;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

/**
 * @author Cosmin Cara
 */
@Repository
@Qualifier(value = "siteRepository")
@Transactional
public interface SiteRepository extends PagingAndSortingRepository<Site, Short> {

    @Query(value = "select id,name,short_name,st_astext(ST_MakeValid(ST_SnapToGrid(cast(geog as geometry), 0.001))) as geog,enabled from site where enabled = true order by id", nativeQuery = true)
    List<Site> getEnabledSites();

    @Query(value = "select id,name,short_name,st_astext(geog) as geog,enabled from site order by name", nativeQuery = true)
    List<Site> getAllSites();

    @Query(value = "select id,name,short_name,st_astext(geog) as geog,enabled from site where short_name=:shortName", nativeQuery = true)
    Site getSiteByShortName(@Param("shortName") String shortName);

    @Query(value = "select id,name,short_name,st_astext(geog) as geog,enabled from site where id=:id", nativeQuery = true)
    Site getSiteById(@Param("id") Short id);

}
