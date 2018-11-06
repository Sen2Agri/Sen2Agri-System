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

import org.apache.commons.io.FileUtils;
import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.DataSourceConfiguration;
import org.esa.sen2agri.entities.ProductTypeInfo;
import org.esa.sen2agri.entities.Satellite;
import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.services.DataSourceService;
import org.esa.sen2agri.services.DownloadService;
import org.esa.sen2agri.services.ProductDownloadListener;
import org.esa.sen2agri.services.ProductTypesService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.eodata.enums.Visibility;
import ro.cs.tao.eodata.metadata.DecodeStatus;
import ro.cs.tao.eodata.metadata.MetadataInspector;
import ro.cs.tao.security.SessionStore;
import ro.cs.tao.services.commons.ControllerBase;
import ro.cs.tao.spi.ServiceRegistryManager;

import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Date;
import java.time.ZoneId;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@Controller
@RequestMapping("/products")
public class ProductController extends ControllerBase {

    @Autowired
    private DataSourceService dataSourceService;
    @Autowired
    private DownloadService downloadService;
    @Autowired
    private PersistenceManager persistenceManager;
    @Autowired
    private ProductTypesService productTypesService;

    @RequestMapping(value = "/disable/{satellite}/{id}", method = RequestMethod.GET)
    public ResponseEntity<List<DataSourceConfiguration>> disable(@PathVariable("satellite") short satelliteId,
                                                                @PathVariable("id") short siteId) {
        downloadService.stop(siteId, satelliteId);
        dataSourceService.setSensorStatus(new SatelliteConverter().convertToEntityAttribute((int)satelliteId),
                                          siteId, false);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @RequestMapping(value = "/enable/{satellite}/{id}", method = RequestMethod.GET)
    public ResponseEntity<List<DataSourceConfiguration>> enable(@PathVariable("satellite") short satelliteId,
                                                                 @PathVariable("id") short siteId) {
        downloadService.start(siteId, satelliteId);
        dataSourceService.setSensorStatus(new SatelliteConverter().convertToEntityAttribute((int)satelliteId),
                                          siteId, true);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @RequestMapping(value = "/enable/status/{satellite}/{id}", method = RequestMethod.GET)
    public ResponseEntity<Boolean> enableStatus(@PathVariable("satellite") short satelliteId,
                                                                 @PathVariable("id") short siteId) {
        return new ResponseEntity<>(dataSourceService.getSensorStatus(
                new SatelliteConverter().convertToEntityAttribute((int)satelliteId),
                siteId), HttpStatus.OK);
    }

    @RequestMapping(value = "/objectstorage/get", method = RequestMethod.GET)
    public ResponseEntity<String[]> getProductTypesForObjectStorage() {
        return new ResponseEntity<>(Config.getSettingValues("scheduled.object.storage.move.product.types",
                                                            new String[0]),
                                            HttpStatus.OK);
    }

    @RequestMapping(value = "/objectstorage/set/{productTypeIds}", method = RequestMethod.GET)
    public ResponseEntity<?> setProductTypesForObjectStorage(@PathVariable String[] productTypeIds) {
        if (productTypeIds != null && productTypeIds.length > 0) {
            Config.setSetting("scheduled.object.storage.move.product.types", String.join(";", productTypeIds));
        }
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @RequestMapping(value = "/types/", method = RequestMethod.GET)
    public ResponseEntity<List<ProductTypeInfo>> getProductTypes() {
        return new ResponseEntity<>(productTypesService.list(), HttpStatus.OK);
    }

    @RequestMapping(value = "/import", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> importProducts(@RequestParam("sourceDir") String sourceDir,
                                            @RequestParam("siteId") short siteId,
                                            @RequestParam("satelliteid") int satelliteId) {
        if (sourceDir == null || siteId <= 0 || satelliteId <= 0) {
            return new ResponseEntity<>("Invalid argument.", HttpStatus.OK);
        } else {
            Site site = persistenceManager.getSiteById(siteId);
            Satellite satellite = new SatelliteConverter().convertToEntityAttribute(satelliteId);
            asyncExecute(() -> { importProductsDelegate(sourceDir, site, satellite); });
            return new ResponseEntity<>("Import request submitted. Please check logs for operation progress.",
                                        HttpStatus.OK);
        }
    }

    private void importProductsDelegate(String sourceDir, Site site, Satellite satellite) {
        Path sourcePath = Paths.get(sourceDir);
        if (Files.exists(sourcePath)) {
            Set<MetadataInspector> services = ServiceRegistryManager.getInstance()
                    .getServiceRegistry(MetadataInspector.class)
                    .getServices();
            MetadataInspector inspector;
            if (services == null) {
                error("No product inspector found");
                return;
            }
            try {
                ProductDownloadListener listener = new ProductDownloadListener(persistenceManager);
                List<Path> folders = Files.walk(sourcePath, 1).collect(Collectors.toList());
                int count = 0;
                DataSourceConfiguration configuration = Config.getDownloadConfiguration(satellite);
                Path downloadPath = Paths.get(configuration.getDownloadPath()).resolve(site.getShortName());
                for (Path folder : folders) {
                    try {
                        if (Files.isDirectory(folder) && !folder.equals(sourcePath)) {
                            Path targetPath;
                            if (!folder.toString().startsWith(downloadPath.toString())) {
                                targetPath = downloadPath.resolve(folder.getFileName());
                                if (!Files.exists(targetPath)) {
                                    debug("Copying %s to %s", folder, targetPath);
                                    FileUtils.copyDirectory(folder.toFile(), targetPath.toFile(), true);
                                }
                            } else {
                                targetPath = folder;
                            }
                            inspector = services.stream()
                                    .filter(i -> DecodeStatus.INTENDED == i.decodeQualification(targetPath))
                                    .findFirst()
                                    .orElse(services.stream()
                                                    .filter(i -> DecodeStatus.SUITABLE == i.decodeQualification(targetPath))
                                                    .findFirst()
                                                    .orElse(null));
                            if (inspector == null) {
                                warn("No suitable metadata inspector found for product %s", targetPath);
                                continue;
                            }
                            MetadataInspector.Metadata metadata = inspector.getMetadata(targetPath);
                            if (metadata != null) {
                                EOProduct product = metadata.toProductDescriptor(targetPath);
                                product.addAttribute("site", String.valueOf(site.getId()));
                                listener.downloadStarted(product);
                                try {
                                    product.setLocation(Paths.get(new URI(product.getLocation())).toString());
                                } catch (Exception ignored) {}
                                product.setEntryPoint(metadata.getEntryPoint());
                                product.setUserName(SessionStore.currentContext().getPrincipal().getName());
                                product.setVisibility(Visibility.PUBLIC);
                                if (metadata.getAquisitionDate() != null) {
                                    product.setAcquisitionDate(Date.from(metadata.getAquisitionDate().atZone(ZoneId.systemDefault()).toInstant()));
                                }
                                if (metadata.getSize() != null) {
                                    product.setApproximateSize(metadata.getSize());
                                }
                                if (metadata.getProductId() != null) {
                                    product.setId(metadata.getProductId());
                                }
                                listener.downloadCompleted(product);
                                count++;
                            }
                        }
                    } catch (Exception e1) {
                        warn("Import for %s failed. Reason: %s", folder, e1.getMessage());
                    }
                }
                info("Imported " + count + " products");
            } catch (Exception e) {
                warn("Error occurred while importing products. Details: " + e.getMessage());
            }
        } else {
            error("Source directory not found");
        }
    }
}
