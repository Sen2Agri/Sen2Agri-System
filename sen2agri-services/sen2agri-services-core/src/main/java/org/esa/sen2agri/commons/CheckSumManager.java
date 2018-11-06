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

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.*;
import java.util.stream.Stream;

public class CheckSumManager {
    private static final CheckSumManager instance;
    private Path fileLocation;
    private final Map<String, Collection<String>> directMap;
    private final Map<String, Collection<String>> inverseMap;

    static {
        instance = new CheckSumManager();
        String value = Config.getProperty("checksum.map.location");
        if (value != null) {
            instance.fileLocation = Paths.get(value);
            if (Files.exists(instance.fileLocation)) {
                try (Stream<String> lines = Files.lines(instance.fileLocation)) {
                    lines.forEach(line -> {
                        String[] tokens = line.split(" ");
                        if (tokens.length > 1) {
                            for (int i = 1; i < tokens.length; i++) {
                                if (!instance.directMap.containsKey(tokens[i])) {
                                    instance.directMap.put(tokens[i], new HashSet<>());
                                }
                                instance.directMap.get(tokens[i]).add(tokens[0]);
                                if (!instance.inverseMap.containsKey(tokens[0])) {
                                    instance.inverseMap.put(tokens[0], new ArrayList<>());
                                }
                                instance.inverseMap.get(tokens[0]).add(tokens[i]);
                            }
                        }
                    });
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        } else {
            instance.fileLocation = null;
        }
    }

    public static CheckSumManager getInstance() { return instance; }

    private CheckSumManager() {
         this.directMap = new HashMap<>();
         this.inverseMap = new HashMap<>();
    }

    public void flush() throws IOException {
        if (fileLocation != null) {
            Path backup = fileLocation.getParent().resolve(fileLocation.getFileName().toString() + ".bak");
            Files.deleteIfExists(backup);
            if (Files.exists(fileLocation)) {
                Files.move(fileLocation, backup);
            } else {
                Files.createFile(fileLocation);
            }
            Set<String> keys = this.inverseMap.keySet();
            for (String key : keys) {
                Collection<String> values = this.inverseMap.get(key);
                if (values != null) {
                    Files.write(fileLocation, (key + " " + String.join(" ", values) + "\n").getBytes(), StandardOpenOption.APPEND);
                }
            }
        }
    }

    public boolean containsChecksum(String checkSum) {
        return this.directMap.containsKey(checkSum);
    }

    public boolean containsProduct(String name) {
        return this.inverseMap.containsKey(name);
    }

    public Collection<String> getProducts(String checksum) {
        return this.directMap.get(checksum);
    }

    public Collection<String> getChecksums(String product) {
        return this.inverseMap.get(product);
    }

    public void put(String key, String value) {
        if (!this.directMap.containsKey(key)) {
            this.directMap.put(key, new HashSet<>());
        }
        this.directMap.get(key).add(value);
        if (!this.inverseMap.containsKey(value)) {
            this.inverseMap.put(value, new ArrayList<>());
        }
        this.inverseMap.get(value).add(key);
    }
}
