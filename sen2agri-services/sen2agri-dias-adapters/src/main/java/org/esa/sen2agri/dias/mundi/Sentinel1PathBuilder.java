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

package org.esa.sen2agri.dias.mundi;

import ro.cs.tao.eodata.EOProduct;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Properties;

/**
 * For MUNDI DIAS, the S1 SLC path is:
 *      s1-l1-slc-YYYY-qq/YYYY/MM/dd/IW/DV/L1-product
 *  where:  qq = the quarter of the year of the product
 *          YYYY = year
 *          MM = month
 *          DD = day
 *          IW = sensor mode (fixed)
 *          DV = dual polarisation (fixed)
 *          L1-product = the product name
 */
public class Sentinel1PathBuilder extends SentinelPathBuilder {

    public Sentinel1PathBuilder(Path repositoryPath, String localPathFormat, Properties properties) {
        super(repositoryPath, localPathFormat, properties);
        List<String> tokens = Arrays.asList(localPathFormat.split("/"));
        this.localPathFormat = String.join("/", tokens.subList(4, 6));
        this.localPathDatePart = String.join("/", tokens.subList(1, 4));
    }

    public Sentinel1PathBuilder(Path repositoryPath, String localPathFormat, Properties properties, boolean testOnly) {
        super(repositoryPath, localPathFormat, properties, testOnly);
        List<String> tokens = Arrays.asList(localPathFormat.split("/"));
        this.localPathFormat = String.join("/", tokens.subList(4, 6));
        this.localPathDatePart = String.join("/", tokens.subList(1, 4));
    }

    @Override
    public Path getProductPath(Path repositoryPath, EOProduct product) {
        // Products are assumed to be organized according to the pattern defined in tao.properties
        Date date = product.getAcquisitionDate();
        final String productName = getProductName(product);
        Path productFolderPath = dateToPath(this.repositoryPath.resolve(getBucketPart(product)), date, this.localPathDatePart);
        productFolderPath = productFolderPath.resolve(this.localPathFormat);
        Path fullProductPath = productFolderPath.resolve(productName);
        logger.fine(String.format("Looking for product %s into %s", product.getName(), fullProductPath));
        if (!this.testOnly && !Files.exists(fullProductPath)) {
            date = product.getProcessingDate();
            if (date != null) {
                productFolderPath = dateToPath(this.repositoryPath.resolve(getBucketPart(product)), date, this.localPathDatePart);
                productFolderPath = productFolderPath.resolve(this.localPathFormat);
                fullProductPath = productFolderPath.resolve(productName);
                logger.fine(String.format("Alternatively looking for product %s into %s", product.getName(), fullProductPath));
                if (!Files.exists(fullProductPath)) {
                    fullProductPath = null;
                }
            } else {
                fullProductPath = null;
            }
        }
        return fullProductPath;
    }
}
