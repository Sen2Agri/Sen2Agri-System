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

import ro.cs.tao.serialization.GenericAdapter;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * @author Cosmin Cara
 */
@XmlRootElement(name = "dsParameter")
public class Parameter {
    private String name;
    private String type;
    private String value;

    public Parameter() { }

    public Parameter(String name, String type, String value) {
        this.name = name;
        this.type = type;
        this.value = value;
    }

    @XmlElement(name = "name")
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    @XmlElement(name = "type")
    public String getType() { return type; }
    public void setType(String type) { this.type = type; }

    @XmlElement(name = "value")
    public String getValue() { return value; }
    public void setValue(String value) { this.value = value; }

    public Object typedValue() throws Exception {
        return String.class.getName().equals(type) ? value :
                new GenericAdapter(type).marshal(value);
    }

    @Override
    public String toString() {
        return "<" + (name != null ? name : "null") + ":" +
                (type != null ? type : "null") + ">=" +
                (value != null ? value : "null");
    }
}
