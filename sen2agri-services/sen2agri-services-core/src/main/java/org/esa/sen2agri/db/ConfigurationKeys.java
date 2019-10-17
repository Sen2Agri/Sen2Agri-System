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

/**
 * @author Cosmin Cara
 */
public final class ConfigurationKeys {
    public static final String SENSOR_STATE = "%s.enabled";
    public static final String DOWNLOAD_DIR = "downloader.%s.write-dir";
    public static final String UPLOAD_DIR = "site.upload-path";
    public static final String DOWNLOADER_ENABLED = "downloader.enabled";
    public static final String DOWNLOADER_SENSOR_ENABLED = "downloader.%s.enabled";
    public static final String DOWNLOADER_SENSOR_FORCE_START = "downloader.%s.forcestart";
    public static final String DOWNLOADER_START_OFFSET = "downloader.start.offset";
    public static final String SKIP_EXISTING_PRODUCTS = "downloader.skip.existing";
    public static final String SCHEDULED_LOOKUP_ENABLED = "scheduled.lookup.enabled";
    public static final String SCHEDULED_RETRY_ENABLED = "scheduled.retry.enabled";
    public static final String MAIL_AUTH = "mail.smtp.auth";
    public static final String MAIL_STARTTLS = "mail.smtp.starttls.enable";
    public static final String MAIL_HOST = "mail.smtp.host";
    public static final String MAIL_PORT = "mail.smtp.port";
    public static final String MAIL_USER = "mail.smtp.username";
    public static final String MAIL_PASSWORD = "mail.smtp.password";
    public static final String MAIL_SENDER = "mail.from";
    public static final String MAIL_RECIPIENT = "mail.to";
    public static final String MAIL_BATCH_LIMIT = "mail.message.batch.limit";

}
