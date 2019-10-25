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

import com.google.common.net.HttpHeaders;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.exception.ExceptionUtils;
import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.esa.sen2agri.db.Constants;
import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.*;
import org.esa.sen2agri.entities.converters.SatelliteConverter;
import org.esa.sen2agri.entities.enums.Satellite;
import org.esa.sen2agri.services.DataSourceService;
import org.esa.sen2agri.services.DownloadService;
import org.esa.sen2agri.services.ProductDownloadListener;
import org.esa.sen2agri.services.ProductTypesService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.util.StreamUtils;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.servlet.mvc.method.annotation.StreamingResponseBody;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.eodata.metadata.DecodeStatus;
import ro.cs.tao.eodata.metadata.MetadataInspector;
import ro.cs.tao.services.commons.ControllerBase;
import ro.cs.tao.services.commons.ResponseStatus;
import ro.cs.tao.services.commons.ServiceResponse;
import ro.cs.tao.spi.ServiceRegistryManager;
import ro.cs.tao.utils.FileUtilities;

import javax.servlet.http.HttpServletResponse;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

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

    @RequestMapping(value = "/", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> getProducts(@RequestParam("userId") int userId,
                                         @RequestParam("siteId") int siteId,
                                         @RequestParam("productTypeId") int productTypeId,
                                         @RequestParam("token") String authToken) {
        if (!RequestContextHolder.currentRequestAttributes().getSessionId().equals(authToken)) {
            return prepareResult(null, HttpStatus.UNAUTHORIZED);
        }
        Map<SiteInfo, Map<String, List<ProductFileInfo>>> products = persistenceManager.getProductInfoBySite(userId, siteId, productTypeId);
        Path path;
        for (Map<String, List<ProductFileInfo>> productType : products.values()) {
            for (List<ProductFileInfo> infos : productType.values()) {
                for (ProductFileInfo info : infos) {
                    if (info.getPath() != null) {
                        path = Paths.get(info.getPath()).toAbsolutePath();
                        try {
                            if (Files.exists(path)) {
                                path = FileUtilities.resolveSymLinks(path);
                                if (Files.isRegularFile(path)) {
                                    info.setSize(Files.size(path));
                                    info.setPath(path.getParent().getFileName().toString());
                                } else if (Files.isDirectory(path)) {
                                    info.setSize(FileUtilities.folderSize(path));
                                    info.setPath(path.getFileName().toString());
                                }
                            } else {
                                info.setPath(null);
                            }
                        } catch (Exception e) {
                            Logger.getLogger(ProductController.class.getName()).warning(String.format("Cannot determine size of path %s. Reason: %s",
                                                                                                      path, ExceptionUtils.getStackTrace(e)));
                        }
                    }
                }
            }
        }
        return prepareResult(products);
    }

    @RequestMapping(value = "/disable/{satellite}/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<DataSourceConfiguration>> disable(@PathVariable("satellite") short satelliteId,
                                                                @PathVariable("id") short siteId) {
        downloadService.stop(siteId, satelliteId);
        dataSourceService.setSensorStatus(new SatelliteConverter().convertToEntityAttribute(satelliteId),
                                          siteId, false);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @RequestMapping(value = "/enable/{satellite}/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<List<DataSourceConfiguration>> enable(@PathVariable("satellite") short satelliteId,
                                                                 @PathVariable("id") short siteId) {
        downloadService.start(siteId, satelliteId);
        dataSourceService.setSensorStatus(new SatelliteConverter().convertToEntityAttribute(satelliteId),
                                          siteId, true);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @RequestMapping(value = "/enable/status/{satellite}/{id}", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<Boolean> enableStatus(@PathVariable("satellite") short satelliteId,
                                                                 @PathVariable("id") short siteId) {
        return new ResponseEntity<>(dataSourceService.getSensorStatus(
                new SatelliteConverter().convertToEntityAttribute(satelliteId),
                siteId), HttpStatus.OK);
    }

    @RequestMapping(value = "/types/", method = RequestMethod.GET)
    public ResponseEntity<List<ProductTypeInfo>> getProductTypes() {
        return new ResponseEntity<>(productTypesService.list(), HttpStatus.OK);
    }

    @RequestMapping(value = "/download", method = RequestMethod.GET, produces = { "application/zip", "application/json" } )
    public ResponseEntity<StreamingResponseBody> download(@RequestParam("siteId") int siteId,
                                                          @RequestParam("name") String productName,
                                                          @RequestParam("token") String authToken,
                                                          HttpServletResponse response) {
        if (!RequestContextHolder.currentRequestAttributes().getSessionId().equals(authToken)) {
            try {
                response.sendError(HttpStatus.UNAUTHORIZED.value());
            } catch (IOException e) {
                error(e.getMessage());
            }
        }
        HighLevelProduct product = persistenceManager.getHighLevelProductByName(siteId, productName);
        if (product == null) {
            try {
                response.sendError(HttpStatus.BAD_REQUEST.value(), String.format("%s does not exist", productName));
            } catch (IOException e) {
                error(e.getMessage());
            }
        } else {
            Path path = Paths.get(product.getFullPath());
            if (Files.exists(path)){
                try {
                    response.setContentType("application/zip");
                    response.setHeader(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=" + FileUtilities.getFilenameWithoutExtension(path) + ".zip");
                    List<Path> paths;
                    if (Files.isDirectory(path)) {
                        paths = FileUtilities.listTree(path);
                    } else {
                        paths = new ArrayList<>();
                        Path mtdPath = Paths.get(path.toString().replace(FileUtilities.getExtension(path), ".mtd"));
                        if (Files.exists(mtdPath)) {
                            paths.add(mtdPath);
                        }
                        paths.add(path);
                        path = path.getParent();
                    }
                    paths.sort((o1, o2) -> {
                        try {
                            return Long.compare(Files.size(o1), Files.size(o2));
                        } catch (IOException e) {
                            return 0;
                        }
                    });
                    final Path rootPaht = path;
                    StreamingResponseBody stream = out -> {
                        try (ZipOutputStream zipOutputStream = new ZipOutputStream(response.getOutputStream())) {
                            for (Path p : paths) {
                                if (Files.isRegularFile(p)) {
                                    String zipPath = rootPaht.relativize(p).toString();
                                    ZipEntry entry = new ZipEntry(zipPath);
                                    entry.setSize(Files.size(p));
                                    entry.setTime(System.currentTimeMillis());
                                    zipOutputStream.putNextEntry(entry);
                                    try (InputStream inputStream = Files.newInputStream(p)) {
                                        StreamUtils.copy(Files.newInputStream(p), zipOutputStream);
                                    }
                                    zipOutputStream.closeEntry();
                                } else {
                                    if (!rootPaht.equals(p)) {
                                        String zipPath = rootPaht.relativize(p).toString() + File.separator;
                                        ZipEntry entry = new ZipEntry(zipPath);
                                        zipOutputStream.putNextEntry(entry);
                                        zipOutputStream.closeEntry();
                                    }
                                }
                            }
                            zipOutputStream.finish();
                        }
                    };
                    return new ResponseEntity<>(stream, HttpStatus.OK);
                } catch (IOException ex) {
                    try {
                        warn(ex.getMessage());
                        response.sendError(HttpStatus.BAD_REQUEST.value(), ex.getMessage());
                    } catch (IOException e) {
                        error(e.getMessage());
                    }
                }
            } else{
                try {
                    response.sendError(HttpStatus.BAD_REQUEST.value(), String.format("Path for product %s does not exist", productName));
                } catch (IOException e) {
                    error(e.getMessage());
                }
            }
        }
        return new ResponseEntity<>(null, HttpStatus.BAD_REQUEST);
    }

    @RequestMapping(value = "/upload", method = RequestMethod.POST, produces = "application/json")
    public ResponseEntity<?> upload(@RequestParam("siteId") int siteId,         // the site
                                    @RequestParam("file") MultipartFile file,   // the file
                                    @RequestParam("folder") String folder) {    // the relative folder to upload to
        ResponseEntity<ServiceResponse<?>> responseEntity;
        try {
            if (file == null || file.getOriginalFilename() == null) {
                throw new NullPointerException("[file]");
            }
            if (file.isEmpty()) {
                throw new IOException("Failed to store empty file " + file.getOriginalFilename());
            }
            if (folder.contains("..") || folder.contains(".")) {
                // This is a security check
                throw new IOException( "Cannot store file with relative path [" + folder + "]");
            }
            Site site = persistenceManager.getSiteById((short) siteId);
            if (site == null) {
                throw new IOException(String.format("Site with id [%d] does not exist", siteId));
            }
            Path uploadPath = Paths.get(Config.getSetting(ConfigurationKeys.UPLOAD_DIR,
                                                          Constants.DEFAULT_UPLOAD_PATH).replace("{user}", site.getShortName().toLowerCase()));
            Path localFolder = uploadPath.resolve(folder);
            Files.createDirectories(localFolder);
            // Resolve filename when coming from IE
            Path filePath = localFolder.resolve(Paths.get(file.getOriginalFilename()).getFileName());
            file.transferTo(filePath.toAbsolutePath().toFile());
            responseEntity = prepareResult("Upload succeeded", ResponseStatus.SUCCEEDED);
        } catch (Exception ex) {
            responseEntity = handleException(ex);
        }
        return responseEntity;
    }

    @RequestMapping(value = "/import/l1", method = RequestMethod.GET, produces = "application/json")
    public ResponseEntity<?> importProducts(@RequestParam("siteId") short siteId,
                                            @RequestParam("folder") String sourceDir,
                                            @RequestParam("satelliteId") short satelliteId,
                                            @RequestParam("link") boolean link) {
        if (sourceDir == null || siteId <= 0 || satelliteId <= 0) {
            return new ResponseEntity<>("Invalid argument.", HttpStatus.OK);
        } else {
            Site site = persistenceManager.getSiteById(siteId);
            Satellite satellite = new SatelliteConverter().convertToEntityAttribute(satelliteId);
            asyncExecute(() -> { importProductsDelegate(sourceDir, site, satellite, link); });
            return new ResponseEntity<>("Import request submitted. Please check logs for operation progress.",
                                        HttpStatus.OK);
        }
    }

    private void importProductsDelegate(String sourceDir, Site site, Satellite satellite, boolean link) {
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
                DataSourceConfiguration configuration = Config.getDownloadConfiguration(site, satellite);
                Path downloadPath = Paths.get(configuration.getDownloadPath()).resolve(site.getShortName());
                for (Path folder : folders) {
                    try {
                        if (Files.isDirectory(folder) && !folder.equals(sourcePath)) {
                            Path targetPath;
                            if (!folder.toString().startsWith(downloadPath.toString())) {
                                targetPath = downloadPath.resolve(folder.getFileName());
                                if (!Files.exists(targetPath)) {
                                    if (link) {
                                        debug("Linking %s to %s", folder, targetPath);
                                        FileUtilities.link(folder, targetPath);
                                    } else {
                                        debug("Copying %s to %s", folder, targetPath);
                                        FileUtils.copyDirectory(folder.toFile(), targetPath.toFile(), true);
                                    }
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
                                Map<String, String> attributes = metadata.getAdditionalAttributes();
                                if (attributes != null) {
                                    for (Map.Entry<String, String> entry : attributes.entrySet()) {
                                        product.addAttribute(entry.getKey(), entry.getValue());
                                    }
                                }
                                listener.downloadStarted(product);
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
