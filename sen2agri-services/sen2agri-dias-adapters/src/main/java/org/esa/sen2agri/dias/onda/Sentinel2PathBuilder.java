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
import ro.cs.tao.products.sentinels.Sentinel2ProductHelper;
import ro.cs.tao.products.sentinels.SentinelProductHelper;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Date;
import java.util.Properties;
import java.util.Scanner;
import java.util.logging.Logger;

/**
 * For ONDA DIAS, the S2 L1C path is:
 *      YYYY/MM/DD/L1C-product-name.zip/L1C-product-name.SAFE
 *  where:  YYYY = year
 *          MM = month
 *          DD = day
 *          L1C-product = the product name (without .SAFE)
 */
public class Sentinel2PathBuilder extends DefaultProductPathBuilder {

    public Sentinel2PathBuilder(Path repositoryPath, String localPathFormat, Properties properties) {
        super(repositoryPath, null, properties);
    }

    public Sentinel2PathBuilder(Path repositoryPath, String localPathFormat, Properties properties, boolean testOnly) {
        super(repositoryPath, localPathFormat, properties, testOnly);
    }

    @Override
    public Path getProductPath(Path repositoryPath, EOProduct product) {
        Path path = null;
        if (product != null) {
            String tileId = product.getAttributeValue("tiles");
            if (tileId == null) {
                Sentinel2ProductHelper helper = (Sentinel2ProductHelper) SentinelProductHelper.create(product.getName());
                tileId = helper.getTileIdentifier();
            }
            if (tileId == null) {
                Logger.getLogger(getClass().getName()).warning(String.format("Cannot determine tileId for product %s",
                                                                             product.getName()));
            } else {
                // Products are assumed to be organized according to the pattern defined in services.properties
                Date date = product.getAcquisitionDate();
                String productName = getProductName(product);
                Path productFolderPath = dateToPath(this.repositoryPath, date, this.localPathFormat);
                path = productFolderPath.resolve(productName.replace(".SAFE", ".zip"))
                                        .resolve(productName);
                logger.fine(String.format("Looking for product %s into %s", product.getName(), path));
                if (!this.testOnly && !Files.exists(path)) {
                    // maybe products are grouped by processing date
                    date = product.getProcessingDate();
                    if (date != null) {
                        productFolderPath = dateToPath(this.repositoryPath, date, this.localPathFormat);
                        path = productFolderPath.resolve(productName.replace(".SAFE", ".zip"))
                                                .resolve(productName);
                        logger.fine(String.format("Alternatively looking for product %s into %s", product.getName(), path));
                        if (!Files.exists(path)) {
                            path = null;
                        }
                    } else {
                        path = null;
                    }
                }
            }
        }
        return path;
    }

    private Path tileToPath(Path path, String tileId) {
        final UTMCodeTokenizer tokenizer = new UTMCodeTokenizer(localPathFormat);
        return path.resolve(tokenizer.getUtmCodePart(tileId))
                    .resolve(tokenizer.getLatBandPart(tileId))
                    .resolve(tokenizer.getSquarePart(tileId));
    }

    private class UTMCodeTokenizer {
        private String utmCode;
        private String latBand;
        private String square;

        UTMCodeTokenizer(String format) {
            utmCode = "";
            latBand = "";
            square = "";
            parse(format);
        }

        String getUtmCodePart(String value) {
            return String.format(utmCode, Integer.parseInt(value.substring(0, value.length() - 3)));
        }

        String getLatBandPart(String value) {
            return String.format(latBand, value.substring(value.length() - 3, value.length() - 2));
        }

        String getSquarePart(String value) {
            return String.format(square, value.substring(value.length() - 2, value.length()));
        }

        @SuppressWarnings("StringConcatenationInLoop")
        private void parse(String format) {
            Scanner scanner = new Scanner(format);
            scanner.useDelimiter("");
            while (scanner.hasNext()) {
                String ch = scanner.next();
                switch (ch) {
                    case "u":
                    case "U":
                        utmCode += ch;
                        break;
                    case "l":
                    case "L":
                        latBand += ch;
                        break;
                    case "s":
                    case "S":
                        square += ch;
                        break;
                    default:
                        break;
                }
            }
            utmCode = "%0" + utmCode.length() + "d";
            latBand = "%." + latBand.length() + "s";
            square = "%." + square.length() + "s";
        }
    }
}
