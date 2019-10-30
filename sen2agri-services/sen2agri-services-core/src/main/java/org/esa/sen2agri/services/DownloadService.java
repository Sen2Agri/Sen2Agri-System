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

import org.esa.sen2agri.commons.DownloadProgress;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.web.beans.Query;
import ro.cs.tao.datasource.ProductStatusListener;
import ro.cs.tao.eodata.EOProduct;

import java.text.ParseException;
import java.util.List;
import java.util.Set;

/**
 * Interface defining methods to controll the downloader service.
 *
 * @author Cosmin Cara
 */
public interface DownloadService {

    long count(short siteId, Query queryObject,
               DataSourceConfiguration configuration) throws ParseException;

    long getCount(short siteId);

    List<SensorProgress> getProgress(short siteId, short satelliteId);

    /**
     * Performs a query, based on the given configuration, for the given site.
     * @param siteId        The site identifier
     * @param queryObject   The query parameters
     * @param configuration The data source configuration
     * @throws ParseException   If the query parameters are not of the expected type
     */
    List<EOProduct> query(short siteId, Query queryObject,
                          DataSourceConfiguration configuration) throws ParseException;

    /**
     * Retrieves the given list of products to a specific path, based on the given configuration.
     * @param siteId        The site identifier
     * @param products      The list of products to be retrieved
     * @param targetPath    The destination path (where the products will be downloaded)
     * @param configuration The data source configuration
     * @return      The list of products, updated with the new location information, if the download was successful.
     */
    List<EOProduct> download(short siteId, List<EOProduct> products, Set<String> tiles, String targetPath,
                             DataSourceConfiguration configuration);
    /**
     * Returns information about the downloads in progress for a specific site.
     * If the <code>siteId</code> parameter value is 0, it returns information about all the downloads in progress.
     * @param siteId    The site identifier
     */
    List<DownloadProgress> getDownloadsInProgress(short siteId);
    /**
     * Stops the downloads and marks the downloader disabled for the specific site.
     * If the <code>siteId</code> parameter value is 0, it stops all the downloads and marks the downloader as
     * disabled for all sites.
     * @param siteId    The site identifier
     */
    void stop(short siteId);
    /**
     * Stops the downloads and marks the downloader disabled for the specific site and specific sensor
     * If the <code>siteId</code> parameter value is 0, it stops all the downloads and marks the downloader as
     * disabled for all sites for the specific sensor.
     * @param siteId    The site identifier
     * @param satelliteId   The satellite identifier
     */
    void stop(short siteId, short satelliteId);
    /**
     * Enables the downloader for the specific site. If the <code>siteId</code> parameter value is 0, it enables
     * the downloader for all the sites.
     * @param siteId    The site identifier
     */
    void start(short siteId);
    /**
     * Enables the downloader for the specific site and sensor. If the <code>siteId</code> parameter value is 0, it enables
     * the downloader for all the sites for the specified sensor.
     * @param siteId    The site identifier
     * @param satelliteId   The satellite identifier
     */
    void start(short siteId, short satelliteId);
    /**
     * Forces the downloader for the specific site to start from the beginning of the first defined season.
     * If the <code>siteId</code> parameter value is 0, it forces the downloader to start from the beginning for all the sites.
     * @param job       The job type
     * @param siteId    The site identifier
     */
    void forceStart(String job, short siteId);
    /**
     * Forces the downloader for the specific site and sensor to start from the beginning of the first defined season.
     * If the <code>siteId</code> parameter value is 0, it forces the downloader to start from the beginning for all the sites
     * and the given sensor.
     * @param job       The job type
     * @param siteId    The site identifier
     * @param satelliteId   The sensor identifier
     */
    void forceStart(String job, short siteId, short satelliteId);

    /**
     * Registers the given product status listener with this instance.
     * @param productStatusListener The listener
     */
    void setProductStatusListener(ProductStatusListener productStatusListener);
}
