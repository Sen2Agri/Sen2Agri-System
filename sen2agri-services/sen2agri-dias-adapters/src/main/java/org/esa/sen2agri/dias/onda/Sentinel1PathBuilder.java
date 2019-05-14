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

package org.esa.sen2agri.dias.onda;

import ro.cs.tao.datasource.DefaultProductPathBuilder;
import ro.cs.tao.eodata.EOProduct;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Date;
import java.util.Properties;

public class Sentinel1PathBuilder extends DefaultProductPathBuilder {

    public Sentinel1PathBuilder(Path repositoryPath, String localPathFormat, Properties properties) {
        super(repositoryPath, localPathFormat, properties);
    }

    public Sentinel1PathBuilder(Path repositoryPath, String localPathFormat, Properties properties, boolean testOnly) {
        super(repositoryPath, localPathFormat, properties, testOnly);
    }

    @Override
    public Path getProductPath(Path repositoryPath, EOProduct product) {
        // Products are assumed to be organized according to the pattern defined in services.properties
        Date date = product.getAcquisitionDate();
        final String productName = getProductName(product);
        Path productFolderPath = dateToPath(this.repositoryPath, date, this.localPathFormat);
        Path fullProductPath = productFolderPath.resolve(productName.replace(".SAFE", ".zip"))
                                                .resolve(productName);
        logger.fine(String.format("Looking for product %s into %s", product.getName(), fullProductPath));
        if (!this.testOnly && !Files.exists(fullProductPath)) {
            date = product.getProcessingDate();
            if (date != null) {
                productFolderPath = dateToPath(this.repositoryPath, date, this.localPathFormat);
                productFolderPath = productFolderPath.resolve(this.localPathFormat);
                fullProductPath = productFolderPath.resolve(productName.replace(".SAFE", ".zip"))
                                                   .resolve(productName);
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
