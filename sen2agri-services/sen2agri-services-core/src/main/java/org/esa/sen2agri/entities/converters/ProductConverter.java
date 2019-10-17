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
package org.esa.sen2agri.entities.converters;

import org.esa.sen2agri.entities.DownloadProduct;
import org.esa.sen2agri.entities.enums.OrbitType;
import org.esa.sen2agri.entities.enums.Satellite;
import ro.cs.tao.datasource.remote.ProductHelper;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.products.sentinels.SentinelProductHelper;
import ro.cs.tao.serialization.GeometryAdapter;

import javax.persistence.AttributeConverter;
import java.net.URISyntaxException;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.Date;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author Cosmin Cara
 */
public class ProductConverter implements AttributeConverter<EOProduct, DownloadProduct> {
    private static final String sciHubPrdExp = "https:\\/\\/scihub.copernicus.eu\\/apihub\\/odata\\/v1\\/Products\\('([a-zA-Z0-9-]+)'\\)\\/\\$value";
    @Override
    public DownloadProduct convertToDatabaseColumn(EOProduct product) {
        DownloadProduct dbProduct = new DownloadProduct();
        String productName = product.getName();
        boolean isSentinel = productName.startsWith("S2") || productName.startsWith("S1");
        dbProduct.setProductName(productName);
        dbProduct.setNbRetries((short)0);
        if (isSentinel) {
            if (!productName.endsWith(".SAFE")) {
                productName += ".SAFE";
                dbProduct.setProductName(productName);
            }
            final ProductHelper helper = SentinelProductHelper.create(productName);
            String orbit = helper.getOrbit();
            dbProduct.setOrbitId(Integer.parseInt(orbit.startsWith("R") ? orbit.substring(1) : orbit));
            if (product.getAttributeValue("orbitdirection") != null) {
                dbProduct.setOrbitType(OrbitType.valueOf(product.getAttributeValue("orbitdirection")));
            }
        } else {
            dbProduct.setOrbitId(-1);
        }
        if (product.getAcquisitionDate() != null) {
            dbProduct.setProductDate(LocalDateTime.ofInstant(product.getAcquisitionDate().toInstant(),
                                                             ZoneId.systemDefault()));
        }
        dbProduct.setSatelliteId(Satellite.valueOf(product.getProductType()));
        dbProduct.setTimestamp(LocalDateTime.now());
        dbProduct.setFullPath(product.getLocation());
        try {
            dbProduct.setFootprint(new GeometryAdapter().marshal(product.getGeometry()));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return dbProduct;
    }

    @Override
    public EOProduct convertToEntityAttribute(DownloadProduct dbProduct) {
        EOProduct eoProduct = new EOProduct();
        String productName = dbProduct.getProductName().replace(".SAFE", "");
        eoProduct.setName(productName);
        eoProduct.setId(productName);
        switch (dbProduct.getSatelliteId()) {
            case Sentinel1:
            case Sentinel2:
                String prodFullName = productName;
                if (!productName.endsWith(".SAFE")) {
                    prodFullName = productName + ".SAFE";
                }
                String prodId;
                Matcher m = Pattern.compile(sciHubPrdExp).matcher(dbProduct.getFullPath());
                if (m.find()) {
                    prodId = m.group(1);
                } else {
                    prodId = prodFullName;
                }
                eoProduct.setId(prodId);
                eoProduct.addAttribute("filename", prodFullName);
                break;
            default:
                    break;
        }
        eoProduct.setProductType(dbProduct.getSatelliteId().name());
        eoProduct.setAcquisitionDate(Date.from(dbProduct.getProductDate().atZone(ZoneId.systemDefault()).toInstant()));
        eoProduct.setGeometry(new GeometryAdapter().unmarshal(dbProduct.getFootprint()));
        try {
            eoProduct.setLocation(dbProduct.getFullPath());
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }
        return eoProduct;

    }
}
