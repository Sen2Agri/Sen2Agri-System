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

import org.esa.sen2agri.entities.enums.Satellite;
import ro.cs.tao.datasource.remote.FetchMode;
import ro.cs.tao.serialization.GenericAdapter;

import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.Properties;
import java.util.stream.Collectors;

/**
 * @author Cosmin Cara
 */
@XmlRootElement(name = "dataSourceConfiguration")
public class DataSourceConfiguration {
    private short id;
    private Short siteId;
    private Satellite satellite;
    private String dataSourceName;
    private int scope;
    private String user;
    private String password;
    private FetchMode fetchMode;
    private int maxRetries;
    private int retryInterval;
    private int maxConnections;
    private String downloadPath;
    private String localArchivePath;
    private boolean enabled;
    private List<Parameter> specificParameters;
    private Integer secondaryDatasourceId;
    private Properties additionalSettings;

    @XmlAttribute(name = "id")
    public short getId() { return id; }
    public void setId(short id) { this.id = id; }

    @XmlAttribute(name = "siteId")
    public Short getSiteId() { return siteId; }
    public void setSiteId(Short siteId) { this.siteId = siteId; }

    @XmlAttribute(name = "name")
    public String getDataSourceName() { return dataSourceName; }
    public void setDataSourceName(String dataSourceName) { this.dataSourceName = dataSourceName; }

    @XmlAttribute(name = "satellite")
    public Satellite getSatellite() { return satellite; }
    public void setSatellite(Satellite satellite) { this.satellite = satellite; }

    @XmlElement(name = "scope")
    public int getScope() { return scope; }
    public void setScope(int scope) { this.scope = scope; }

    @XmlElement(name = "user")
    public String getUser() { return user; }
    public void setUser(String user) { this.user = user; }

    @XmlElement(name = "password")
    public String getPassword() { return password; }
    public void setPassword(String password) { this.password = password; }

    @XmlElement(name = "fetchMode")
    public FetchMode getFetchMode() { return fetchMode; }
    public void setFetchMode(FetchMode fetchMode) { this.fetchMode = fetchMode; }

    @XmlElement(name = "maxRetries")
    public int getMaxRetries() { return maxRetries; }
    public void setMaxRetries(int maxRetries) { this.maxRetries = maxRetries; }

    @XmlElement(name = "retryInterval")
    public int getRetryInterval() { return retryInterval; }
    public void setRetryInterval(int retryInterval) { this.retryInterval = retryInterval; }

    @XmlElement(name = "maxConnections")
    public int getMaxConnections() { return maxConnections; }
    public void setMaxConnections(int maxConnections) { this.maxConnections = maxConnections; }

    @XmlElement(name = "targetPath")
    public String getDownloadPath() { return downloadPath; }
    public void setDownloadPath(String downloadPath) { this.downloadPath = downloadPath; }

    @XmlElement(name = "localArchivePath")
    public String getLocalArchivePath() { return localArchivePath; }
    public void setLocalArchivePath(String localArchivePath) { this.localArchivePath = localArchivePath; }

    @XmlElement(name = "enabled")
    public boolean isEnabled() { return enabled; }
    public void setEnabled(boolean enabled) { this.enabled = enabled; }

    @XmlElementWrapper(name = "specificParameters")
    @XmlElement(name = "dsParameter")
    public List<Parameter> getSpecificParameters() { return specificParameters; }
    public void setSpecificParameters(List<Parameter> specificParameters) { this.specificParameters = specificParameters; }

    @XmlElement(name = "secondaryDatasourceId")
    public Integer getSecondaryDatasourceId() { return secondaryDatasourceId; }
    public void setSecondaryDatasourceId(Integer secondaryDatasourceId) { this.secondaryDatasourceId = secondaryDatasourceId; }

    public Object getParameterValue(String name) throws Exception {
        if (specificParameters != null) {
            Optional<Parameter> parameter = specificParameters.stream().filter(p -> p.getName().equals(name)).findFirst();
            return parameter.isPresent() ?
                    new GenericAdapter(parameter.get().getType()).marshal(parameter.get().getValue()) :
                    null;
        } else {
            return null;
        }
    }

    public <T> void setParameter(String name, Class<T> tClass, T value) {
        if (specificParameters == null) {
            specificParameters = new ArrayList<>();
        }
        specificParameters.add(new Parameter(name, tClass.getTypeName(), String.valueOf(value)));
    }

    public Properties getAdditionalSettings() { return additionalSettings; }
    public void setAdditionalSettings(Properties additionalSettings) {
        this.additionalSettings = additionalSettings;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        DataSourceConfiguration that = (DataSourceConfiguration) o;
        return getSatellite() == that.getSatellite() && (getDataSourceName() != null ? getDataSourceName().equals(that.getDataSourceName()) : that.getDataSourceName() == null);
    }

    @Override
    public int hashCode() {
        int result = getSatellite() != null ? getSatellite().hashCode() : 0;
        result = 31 * result + (getDataSourceName() != null ? getDataSourceName().hashCode() : 0);
        return result;
    }

    @Override
    public String toString() {
        return "DataSourceConfiguration{" +
                "id=" + id +
                ", satellite=" + (satellite != null ? satellite.name() : "null") +
                ", dataSourceName=" + (dataSourceName != null ? '\'' + dataSourceName + '\'' : "null") +
                ", scope=" + scope +
                ", user=" + (user != null ? '\'' + user + '\'' : "null") +
                ", password=" + (password != null ? '\'' + password + '\'' : "null") +
                ", fetchMode=" + (fetchMode != null ? fetchMode.name() : "null") +
                ", maxRetries=" + maxRetries +
                ", retryInterval=" + retryInterval +
                ", maxConnections=" + maxConnections +
                ", downloadPath=" + (downloadPath != null ? '\'' + downloadPath + '\'' : "null") +
                ", localArchivePath=" + (localArchivePath != null ? '\'' + localArchivePath + '\'' : "null") +
                ", enabled=" + enabled +
                ", secondaryDatasourceId=" + (secondaryDatasourceId != null ? secondaryDatasourceId : "null") +
                ", specificParameters=[" + (specificParameters != null ?
                specificParameters.stream()
                                  .map(Object::toString)
                                  .collect(Collectors.joining(",")) : "null") + "]" +
                ", additionalSettings=" + (additionalSettings != null ? additionalSettings.toString() : "null") +
                '}';
    }
}
