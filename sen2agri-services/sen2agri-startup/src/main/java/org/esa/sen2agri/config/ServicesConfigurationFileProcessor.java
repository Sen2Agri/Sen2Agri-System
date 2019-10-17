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

package org.esa.sen2agri.config;

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.db.Database;
import ro.cs.tao.services.commons.config.ConfigurationFileProcessor;

import java.nio.file.Path;
import java.util.Properties;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

public class ServicesConfigurationFileProcessor implements ConfigurationFileProcessor {

    @Override
    public String getFileName() { return "services.properties"; }

    @Override
    public String getFileResourceLocation() { return "/config/services.properties"; }

    @Override
    public void performAdditionalConfiguration(Path configDirectory, Properties properties) {
        Config.setFileConfiguration(properties);
        final String logLevel = Config.getProperty("log.level");
        if (logLevel != null) {
            Logger rootLogger = LogManager.getLogManager().getLogger("");
            Handler[] handlers = rootLogger.getHandlers();
            Level newLevel = Level.parse(logLevel);
            rootLogger.setLevel(newLevel);
            for (Handler h : handlers) {
                h.setLevel(newLevel);
            }
        }
        try {
            Database.checkDatasource();
            Database.checkConfig();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
        try {
            Database.checkProductStats();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}
