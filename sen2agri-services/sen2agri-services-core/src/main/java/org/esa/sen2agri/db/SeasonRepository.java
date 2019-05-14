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

import org.esa.sen2agri.entities.Season;
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
@Qualifier(value = "seasonRepository")
@Transactional
public interface SeasonRepository extends PagingAndSortingRepository<Season, Short> {
    @Query(value = "select * from season where enabled = true order by id", nativeQuery = true)
    List<Season> getEnabledSeasons();

    @Query(value = "select * from season where site_id = :siteId and enabled = true order by id", nativeQuery = true)
    List<Season> getEnabledSeasons(@Param("siteId") short siteId);

    @Query(value = "select * from season order by id", nativeQuery = true)
    List<Season> getSeasons();

    @Query(value = "select * from season where site_id = :siteId order by id", nativeQuery = true)
    List<Season> getSeasons(@Param("siteId") short siteId);
}
