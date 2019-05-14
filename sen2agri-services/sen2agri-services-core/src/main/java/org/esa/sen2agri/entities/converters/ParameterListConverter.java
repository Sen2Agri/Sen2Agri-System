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
package org.esa.sen2agri.entities.converters;

import org.esa.sen2agri.entities.Parameter;
import ro.cs.tao.serialization.BaseSerializer;
import ro.cs.tao.serialization.MediaType;
import ro.cs.tao.serialization.SerializationException;
import ro.cs.tao.serialization.SerializerFactory;

import javax.persistence.AttributeConverter;
import javax.xml.transform.stream.StreamSource;
import java.io.StringReader;
import java.util.List;

/**
 * @author Cosmin Cara
 */
public class ParameterListConverter implements AttributeConverter<List<Parameter>, String> {
    private static final BaseSerializer<Parameter> serializer;

    static {
        try {
            serializer = SerializerFactory.create(Parameter.class, MediaType.JSON);
        } catch (SerializationException e) {
            throw new RuntimeException(e.getCause());
        }
    }

    @Override
    public String convertToDatabaseColumn(List<Parameter> parameters) {
        try {
            return serializer.serialize(parameters, "parameters");
        } catch (SerializationException e) {
            throw new RuntimeException(e.getCause());
        }
    }

    @Override
    public List<Parameter> convertToEntityAttribute(String s) {
        try {
            return serializer.deserializeList(Parameter.class, new StreamSource(new StringReader(s)));
        } catch (SerializationException e) {
            throw new RuntimeException(e.getCause());
        }
    }
}
