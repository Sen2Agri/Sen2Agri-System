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

import java.nio.file.Path;
import java.util.Calendar;
import java.util.Properties;

abstract class SentinelPathBuilder extends DefaultProductPathBuilder {
    String localPathDatePart;
    private String bucketPart;

    SentinelPathBuilder(Path repositoryPath, String localPathFormat, Properties properties) {
        super(repositoryPath, null, properties);
        this.bucketPart = localPathFormat.substring(0, localPathFormat.indexOf('/'));
    }

    SentinelPathBuilder(Path repositoryPath, String localPathFormat, Properties properties, boolean testOnly) {
        super(repositoryPath, null, properties, testOnly);
        this.bucketPart = localPathFormat.substring(0, localPathFormat.indexOf('/'));
    }

    String getBucketPart(EOProduct product) {
        final Calendar calendar = Calendar.getInstance();
        calendar.setTime(product.getAcquisitionDate());
        final int year = calendar.get(Calendar.YEAR);
        final int month = calendar.get(Calendar.MONTH) + 1;
        final int quarter = month < 4 ? 1 : month < 7 ? 2 : month < 10 ? 3 : 4;
        return this.bucketPart.replace("YYYY", String.valueOf(year)).replace("qq", "q" + quarter);
    }
}
