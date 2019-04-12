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

package org.esa.sen2agri.entities;

public class ProductDetails {
    private int id;
    private double minValue;
    private double maxValue;
    private double meanValue;
    private double stdDevValue;
    private Integer[] histogram;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public double getMinValue() {
        return minValue;
    }

    public void setMinValue(double minValue) {
        this.minValue = minValue;
    }

    public double getMaxValue() {
        return maxValue;
    }

    public void setMaxValue(double maxValue) {
        this.maxValue = maxValue;
    }

    public double getMeanValue() {
        return meanValue;
    }

    public void setMeanValue(double meanValue) {
        this.meanValue = meanValue;
    }

    public double getStdDevValue() {
        return stdDevValue;
    }

    public void setStdDevValue(double stdDevValue) {
        this.stdDevValue = stdDevValue;
    }

    public Integer[] getHistogram() {
        return histogram;
    }

    public void setHistogram(Integer[] histogram) {
        this.histogram = histogram;
    }
}
