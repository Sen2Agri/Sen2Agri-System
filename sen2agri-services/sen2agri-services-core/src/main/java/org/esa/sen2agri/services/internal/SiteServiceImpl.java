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
package org.esa.sen2agri.services.internal;

import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.HighLevelProduct;
import org.esa.sen2agri.entities.Season;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.ProductType;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.services.ScheduleManager;
import org.esa.sen2agri.services.SiteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.File;
import java.io.IOException;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Logger;

/**
 * @author Cosmin Cara
 */
@Service("siteService")
public class SiteServiceImpl implements SiteService {

    @Autowired
    private PersistenceManager persistenceManager;

    @Autowired
    private ScheduleManager scheduleManager;

    private Logger logger = Logger.getLogger(SiteService.class.getName());

    @Override
    public List<Site> list()
    {
        return persistenceManager.getAllSites();
    }

    @Override
    public int createSite(String siteName, String zipFileName, boolean enabled) {
        return -1;
    }

    @Override
    public void deleteSite(short siteId, ProductType... typesToDelete) {
        ProductType[] types = typesToDelete;
        if (types != null) {
            Set<String> foldersToDelete = new HashSet<>();
            final List<HighLevelProduct> products = persistenceManager.getProducedProducts(siteId, types);
            for (HighLevelProduct product : products) {
                try {
                    deleteDirectory(product.getFullPath());
                    // TODO: Here is a little bit more complicated to delete the containing folders
                    //      due to the configuration options
                    // foldersToDelete.add(getProductContainingFolder(product.getFullPath()));
                } catch (IOException ex) {
                    logger.warning(String.format("Failed to delete '%s': %s", product.getProductName(), ex.getMessage()));
                }
            }

            for (Satellite satellite : Satellite.values()) {
                final List<DownloadProduct> downloadedProducts = persistenceManager.getDownloadedProducts(siteId, satellite);
                for (DownloadProduct product : downloadedProducts) {
                    try {
                        deleteDirectory(product.getFullPath());
                        foldersToDelete.add(getProductContainingFolder(product.getFullPath()));
                    } catch (IOException ex) {
                        logger.warning(String.format("Failed to delete '%s': %s", product.getProductName(), ex.getMessage()));
                    }
                }
            }
            deleteContainingSiteFolders(foldersToDelete, siteId);
        }

        persistenceManager.deleteSite(siteId);
        //Config.siteDeleted(siteId);
        //scheduleManager.refresh();
    }

    @Override
    public short getSiteId(String siteShortName) {
        return persistenceManager.getSiteByShortName(siteShortName).getId();
    }

    @Override
    public List<Season> getSiteSeason(String id) {
        try {
            Short seasonId = Short.parseShort(id);
            return this.persistenceManager.getSeasons(seasonId);
        } catch (NumberFormatException nfe) {
            Site site = this.persistenceManager.getSiteByShortName(id);
            return this.persistenceManager.getSeasons(site.getId());
        }
    }

    private void deleteDirectory(String directory) throws IOException {
        Path dirPath = Paths.get(directory);
        Files.walk(dirPath, FileVisitOption.FOLLOW_LINKS)
                .sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
    }

    private String getProductContainingFolder(String productFullPath) {
        return new File(productFullPath).getParent();
    }

    private void deleteContainingSiteFolders(Set<String> foldersToDelete, short siteId) {
        String siteShortName = persistenceManager.getSiteById(siteId).getShortName();
        for (String folder: foldersToDelete) {
            if (folder.endsWith(siteShortName)) {
                try {
                    deleteDirectory(folder);
                } catch (IOException ex) {
                    logger.warning(String.format("Failed to delete site folder '%s': %s", folder, ex.getMessage()));
                }
            }
        }
    }
}
