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

import ro.cs.tao.datasource.DefaultProductPathBuilder;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.products.landsat.Landsat8ProductHelper;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;

public class Landsat8PathBuilder extends DefaultProductPathBuilder {
    private String localPathDatePart;

    public Landsat8PathBuilder(Path repositoryPath, String localPathFormat, Properties properties) {
        super(repositoryPath, null, properties);
        List<String> tokens = Arrays.asList(localPathFormat.split("/"));
        this.localPathFormat = String.join("/", tokens.subList(0, 2));
        this.localPathDatePart = String.join("/", tokens.subList(2, 4));
    }

    public Landsat8PathBuilder(Path repositoryPath, String localPathFormat, Properties properties, boolean testOnly) {
        super(repositoryPath, null, properties, testOnly);
        List<String> tokens = Arrays.asList(localPathFormat.split("/"));
        this.localPathFormat = String.join("/", tokens.subList(0, 2));
        this.localPathDatePart = String.join("/", tokens.subList(2, 4));
    }

    @Override
    public Path getProductPath(Path repositoryPath, EOProduct product) {
        Path path = null;
        if (product != null) {
            Landsat8ProductHelper helper = new Landsat8ProductHelper(product.getName());
            String tileId = helper.getPath() + helper.getRow();
            Date date = product.getAcquisitionDate();
            String productName = getProductName(product);
            Path productFolderPath = pathRowToPath(this.repositoryPath, tileId);
            productFolderPath = dateToPath(productFolderPath, date, this.localPathDatePart);
            path = productFolderPath.resolve(productName);
            logger.fine(String.format("Looking for product %s into %s", product.getName(), path));
            if (!this.testOnly && !Files.exists(path)) {
                // maybe products are grouped by processing date
                date = product.getProcessingDate();
                if (date != null) {
                    productFolderPath = dateToPath(pathRowToPath(this.repositoryPath, tileId), date, this.localPathDatePart);
                    path = productFolderPath.resolve(productName);
                    logger.fine(String.format("Alternatively looking for product %s into %s", product.getName(), path));
                    if (!Files.exists(path)) {
                        path = null;
                    }
                } else {
                    path = null;
                }
            }
        }
        return path;
    }

    private Path pathRowToPath(Path path, String pathRow) {
        final PathRowTokenizer tokenizer = new PathRowTokenizer(localPathFormat);
        return path.resolve(tokenizer.getPathPart(pathRow))
                .resolve(tokenizer.getRowPart(pathRow));
    }

    private class PathRowTokenizer {
        private String path;
        private String row;

        PathRowTokenizer(String format) {
            path = "";
            row = "";
            parse(format);
        }

        String getPathPart(String pathRow) { return String.format(path, Integer.parseInt(pathRow.substring(0, 3))); }

        String getRowPart(String pathRow) { return String.format(row, Integer.parseInt(pathRow.substring(3))); }

        @SuppressWarnings("StringConcatenationInLoop")
        private void parse(String format) {
            Scanner scanner = new Scanner(format);
            scanner.useDelimiter("");
            while (scanner.hasNext()) {
                String ch = scanner.next();
                switch (ch) {
                    case "p":
                    case "P":
                        path += ch;
                        break;
                    case "r":
                    case "R":
                        row += ch;
                        break;
                    default:
                        break;
                }
            }
            path = "%0" + path.length() + "d";
            row = "%0" + row.length() + "d";
        }
    }
}
