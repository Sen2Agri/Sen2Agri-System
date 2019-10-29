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

package org.esa.sen2agri.commons;

import ro.cs.tao.messaging.Topic;

public class ProcessingTopic extends Topic {
    public static final Topic PROCESSING_ATTENTION = Topic.create("processing", "attention");
    public static final Topic PROCESSING_WARNING = Topic.create("processing", "warning");
    public static final Topic PROCESSING_ERROR = Topic.create("processing", "error");
    public static final Topic PROCESSING_COMPLETED = Topic.create("processing", "completed");;
    public static final Topic COMMAND = Topic.create("command");

    private ProcessingTopic(String category, String tag) {
        super(category, tag);
    }
}
