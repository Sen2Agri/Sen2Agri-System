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

package org.esa.sen2agri.scheduling;

import org.esa.sen2agri.entities.Site;
import org.esa.sen2agri.entities.enums.Satellite;
import ro.cs.tao.eodata.EOProduct;
import ro.cs.tao.utils.Triple;

import java.util.List;
import java.util.function.Consumer;
import java.util.logging.Logger;
import java.util.stream.Collectors;

public class DownloadTask implements Runnable {

    private final Logger parentLogger;
    private final Site site;
    private final Satellite satellite;
    private final Runnable runnable;
    private final List<String> productNames;
    private final Consumer<Triple<String, String, String>> callback;

    DownloadTask(Logger logger, Site site, Satellite sat, List<EOProduct> products, Runnable runnable, Consumer<Triple<String, String, String>> callback) {
        this.parentLogger = logger;
        this.site = site;
        this.satellite = sat;
        this.productNames = products.stream().map(p -> p.getName() != null ? p.getName() : p.getId()).collect(Collectors.toList());
        this.runnable = runnable;
        this.callback = callback;
    }

    @Override
    public void run() {
        try {
            this.parentLogger.finest(String.format("A task for '%s' has been dequeued for execution",
                                                   site.getShortName() + "-" + satellite.friendlyName()));
            this.runnable.run();
        } catch (Exception ex) {
            this.parentLogger.warning(ex.getMessage());
        } finally {
            Triple<String, String, String> key = new Triple<>(site.getName(),
                                                              satellite.friendlyName(),
                                                              String.join(",", productNames));
            try {
                this.callback.accept(key);
            } catch (Throwable t) {
                this.parentLogger.severe(String.format("Exception when completing DownloadTask: %s", t.getMessage()));
            }
        }
    }
}
