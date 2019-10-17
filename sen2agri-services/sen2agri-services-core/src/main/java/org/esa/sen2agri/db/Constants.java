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
package org.esa.sen2agri.db;

import ro.cs.tao.datasource.remote.FetchMode;

/**
 * @author Cosmin Cara
 */
public class Constants {
    static final int DEFAULT_RETRY_INTERVAL = 60;
    static final int DEFAULT_MAX_RETRIES = 72;
    static final FetchMode DEFAULT_FETCH_MODE = FetchMode.OVERWRITE;
    static final int DEFAULT_SCOPE = 0x03;
    public static final String DEFAULT_TARGET_PATH = "/mnt/archive";
    public static final String DEFAULT_UPLOAD_PATH = "/mnt/upload/{user}";
    static final int DEFAULT_MAX_CONNECTIONS = 1;
}
