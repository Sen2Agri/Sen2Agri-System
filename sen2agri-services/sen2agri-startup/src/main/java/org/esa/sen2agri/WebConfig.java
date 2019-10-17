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

package org.esa.sen2agri;

import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;
import ro.cs.tao.configuration.ConfigurationManager;
import ro.cs.tao.services.commons.WebConfiguration;

import java.nio.file.Path;
import java.nio.file.Paths;

@EnableWebMvc
@Configuration
public class WebConfig extends WebConfiguration {
    @Override
    protected String siteURIPath() {
        final String siteBase = ConfigurationManager.getInstance().getValue("site.location", "static");
        Path sitePath = Paths.get(siteBase);
        if (!sitePath.isAbsolute()) {
            sitePath = ServicesStartup.homeDirectory().resolve(siteBase).toAbsolutePath();
        }
        ConfigurationManager.getInstance().setValue("site.path", sitePath.toString());
        final String siteURI = sitePath.toUri().toString();
        ConfigurationManager.getInstance().setValue("site.url", siteURI);
        return siteURI;
    }

    @Override
    protected String sitePrefix() { return "/ui"; }

    @Override
    protected String defaultPage() { return "login.html"; }
}
