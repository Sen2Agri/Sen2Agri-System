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
package org.esa.sen2agri.web.beans;

import ro.cs.tao.component.converters.ConverterFactory;
import ro.cs.tao.component.converters.ParameterConverter;
import ro.cs.tao.datasource.converters.ConversionException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Cosmin Cara
 */
public class Query {
    private String user;
    private String password;
    private Map<String, Object> values;

    public Query() { }

    public Map<String, Object> getValues() { return values; }

    public void setValues(Map<String, Object> values) { this.values = values; }

    public String getUser() { return user; }

    public void setUser(String user) { this.user = user; }

    public String getPassword() { return password; }

    public void setPassword(String password) { this.password = password; }

    public boolean hasParameter(String parameterName) {
        return this.values != null && this.values.containsKey(parameterName);
    }

    public List<Query> splitByParameter(String parameterName) throws ConversionException {
        List<Query> subQueries = null;
        Object parameterValue;
        if (this.values != null && ((parameterValue = this.values.get(parameterName)) != null)) {
            subQueries = new ArrayList<>();
            String value = parameterValue.toString();
            ParameterConverter converter = ConverterFactory.getInstance().create(parameterValue.getClass());
            if (value.startsWith("[") && value.endsWith("]")) {
                String[] vals = value.substring(1, value.length() - 1).split(",");
                for (String val : vals) {
                    Query subQuery = new Query();
                    subQuery.user = this.user;
                    subQuery.password = this.password;
                    subQuery.values = new HashMap<>();
                    for (Map.Entry<String, Object> entry : this.values.entrySet()) {
                        if (!parameterName.equals(entry.getKey())) {
                            subQuery.values.put(entry.getKey(), entry.getValue());
                        }
                    }
                    subQuery.values.put(parameterName, converter.fromString(val));
                    subQueries.add(subQuery);
                }
            } else {
                subQueries.add(this);
            }
        }
        return subQueries;
    }
}
