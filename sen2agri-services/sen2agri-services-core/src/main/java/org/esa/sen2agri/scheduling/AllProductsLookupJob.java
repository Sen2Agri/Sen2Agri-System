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

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.entities.Season;
import org.esa.sen2agri.entities.Site;

import java.time.LocalDateTime;
import java.util.List;

/**
 * @author Cosmin Udroiu
 */
public class AllProductsLookupJob extends LookupJob {

    public AllProductsLookupJob() {
        super();
    }

    @Override
    public String groupName() { return "AllProductsLookup"; }

    @Override
    public String configKey() {
        return "scheduled.lookup.all_products.enabled";
    }

    @Override
    protected LocalDateTime getStartDate(Satellite satellite, Site site, List<Season> seasons) {
        LocalDateTime minStartDate = seasons.get(0).getStartDate().atStartOfDay();
        int downloaderStartOffset = Config.getAsInteger(ConfigurationKeys.DOWNLOADER_START_OFFSET, 1);
        if (downloaderStartOffset > 0) {
            minStartDate = minStartDate.minusMonths(downloaderStartOffset);
        }
        return minStartDate;
    }
}
