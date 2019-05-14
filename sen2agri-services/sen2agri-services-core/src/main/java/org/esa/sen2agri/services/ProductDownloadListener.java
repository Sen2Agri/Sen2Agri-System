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

import org.apache.commons.lang3.exception.ExceptionUtils;
import org.esa.sen2agri.commons.CheckSumManager;
import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.OrbitType;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.entities.Status;
import org.esa.sen2agri.entities.converters.ProductConverter;
import ro.cs.tao.datasource.ProductStatusListener;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.eodata.metadata.MetadataInspector;
import ro.cs.tao.products.sentinels.Sentinel1MetadataInspector;
import ro.cs.tao.serialization.GeometryAdapter;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.util.HashSet;
import java.util.Set;
import java.util.logging.Logger;

/**
 * @author Cosmin Cara
 */
public class ProductDownloadListener implements ProductStatusListener {

    private PersistenceManager persistenceManager;
    private boolean checkMd5;
    private Logger logger = Logger.getLogger(ProductDownloadListener.class.getName());

    public ProductDownloadListener(PersistenceManager persistenceManager) {
        this.persistenceManager = persistenceManager;
        this.checkMd5 = Config.getProperty("checksum.map.location", null) != null;
    }

    @Override
    public boolean downloadStarted(EOProduct product) {
        DownloadProduct dbProduct = getDbProduct(product);
        if (dbProduct == null) {
            dbProduct = new ProductConverter().convertToDatabaseColumn(product);
        } else {
            if (dbProduct.getFootprint() == null && product.getGeometry() != null) {
                try {
                    dbProduct.setFootprint(new GeometryAdapter().marshal(product.getGeometry()));
                } catch (Exception e) {
                    logger.warning(String.format("Cannot update fooprint for product %s. Reason: %s",
                                                 product.getName(), e.getMessage()));
                }
            }
            if (dbProduct.getOrbitType() == null && product.getAttributeValue("orbitdirection") != null) {
                dbProduct.setOrbitType(OrbitType.valueOf(product.getAttributeValue("orbitdirection")));
            }
            dbProduct = persistenceManager.save(dbProduct);
            // avoid redownloading the products aborted, downloaded or processed
            if (!Status.DOWNLOADING.equals(dbProduct.getStatusId()) &&
                    !Status.FAILED.equals(dbProduct.getStatusId())) {
                return false;
            }
        }
        String site = product.getAttributeValue("site");
        if (site != null) {
            dbProduct.setSiteId(Short.parseShort(site));
        }
        //if (dbProduct.getStatusId() == Status.FAILED) {
        dbProduct.setNbRetries((short) (dbProduct.getNbRetries() + 1));
        //}
        dbProduct.setStatusId(Status.DOWNLOADING);
        dbProduct.setTimestamp(LocalDateTime.now());
        persistenceManager.save(dbProduct);
        return true;
    }

    @Override
    public void downloadCompleted(EOProduct product) {
        DownloadProduct dbProduct = getDbProduct(product);
        if (dbProduct != null) {
            String site = product.getAttributeValue("site");
            if (site != null) {
                dbProduct.setSiteId(Short.parseShort(site));
            }
            String tiles = product.getAttributeValue("tiles");
            if (tiles != null) {
                dbProduct.setTiles(tiles.split(","));
            }
            dbProduct.setStatusId(Status.DOWNLOADED);
            boolean duplicate = false;
            try {
                String path = product.getLocation();
                Path productPath;
                if (!path.contains(dbProduct.getProductName()) &&
                        (!path.toLowerCase().endsWith(".zip") || !path.toLowerCase().endsWith(".tar.gz"))) {
                    productPath = Paths.get(new URI(path)).resolve(dbProduct.getProductName());
                } else {
                    // hack for Sentinel-1 to exclude products with the same MD5 sums for the rasters
                    if (this.checkMd5 && Satellite.Sentinel1.name().equals(product.getProductType())) {
                        MetadataInspector inspector = new Sentinel1MetadataInspector();
                        try {
                            MetadataInspector.Metadata metadata = inspector.getMetadata(Paths.get(path));
                            Set<String> md5 = metadata.getControlSums();
                            Set<String> existingProducts = new HashSet<>();
                            if (md5.size() > 0) {
                                final CheckSumManager verifier = CheckSumManager.getInstance();
                                md5.forEach(m -> {
                                                     if (verifier.containsChecksum(m)) {
                                                         existingProducts.addAll(verifier.getProducts(m));
                                                     }
                                                     verifier.put(m, product.getName());
                                                 });
                                duplicate = existingProducts.size() >= 1;
                                if (duplicate) {
                                    logger.warning(String.format("Product %s has equal rasters with products %s",
                                                                 product.getName(), String.join(",", existingProducts)));
                                }
                                verifier.flush();
                            }
                        } catch (IOException e) {
                            logger.severe(String.format("Cannot open product %s metadata. Reason: %s",
                                                        product.getName(), ExceptionUtils.getStackTrace(e)));
                        }
                    }
                    productPath = Paths.get(new URI(path));
                }
                dbProduct.setFullPath(productPath.toString());
            } catch (URISyntaxException e) {
                e.printStackTrace();
                logger.warning(String.format("Error setting the location for product %s. The error was %s", product.getName(),
                        e.getMessage()));
            }
            dbProduct.setTimestamp(LocalDateTime.now());
            dbProduct.setStatusReason(null);
            if (!duplicate) {
                persistenceManager.save(dbProduct);
            }
        }
    }

    @Override
    public void downloadFailed(EOProduct product, String reason) {
        handleDownloadUnsuccessful(product, reason, Status.FAILED);
    }

    @Override
    public void downloadAborted(EOProduct product, String reason) {
        handleDownloadUnsuccessful(product, reason, Status.ABORTED);
    }

    @Override
    public void downloadIgnored(EOProduct product, String reason) {
        handleDownloadUnsuccessful(product, reason, Status.IGNORED);
    }

    private void handleDownloadUnsuccessful(EOProduct product, String reason, Status status) {
        DownloadProduct dbProduct = getDbProduct(product);
        if (dbProduct == null) {
            dbProduct = new ProductConverter().convertToDatabaseColumn(product);
        }
        short site = getSiteId(product);
        if (site != 0) {
            dbProduct.setSiteId(site);
        }
        String tiles = product.getAttributeValue("tiles");
        if (tiles != null) {
            dbProduct.setTiles(tiles.split(","));
        }
        String maxRetries = product.getAttributeValue("maxRetries");
        if (status == Status.FAILED && (maxRetries != null && !"".equals(maxRetries.trim())) &&
                (dbProduct.getNbRetries() >= Integer.parseInt(maxRetries))) {
            status = Status.ABORTED;
            logger.info(String.format("Product %s aborted after %s retries", product.getName(),
                    maxRetries));
        }

        dbProduct.setStatusId(status);
        dbProduct.setStatusReason(reason);
        dbProduct.setTimestamp(LocalDateTime.now());
        persistenceManager.save(dbProduct);
    }

    private DownloadProduct getDbProduct(EOProduct product) {
        DownloadProduct dbProduct = null;
        String productName = product.getName();
        // TODO: This should be somehow unified with the code from ProductConverter
        boolean isSentinel = productName.startsWith("S2") || productName.startsWith("S1");
        if (isSentinel) {
            if (!productName.endsWith(".SAFE")) {
                productName += ".SAFE";
            }
        }
        short siteId = getSiteId(product);
        try {
            dbProduct = persistenceManager.getProductByName(siteId, productName);
            if (isSentinel && product.getAttributeValue("orbitdirection") != null &&
                    dbProduct != null && dbProduct.getOrbitType() == null) {
                dbProduct.setOrbitType(OrbitType.valueOf(product.getAttributeValue("orbitdirection")));
            }
        } catch (Exception e) {
            logger.warning(String.format("Unsuccessful database query for %s. Reason: %s",
                                         productName, e.getMessage()));
        }
        return dbProduct;
    }

    private short getSiteId(EOProduct product) {
        String site = product.getAttributeValue("site");
        if (site != null) {
            return Short.parseShort(site);
        }
        return 0;
    }
}
