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
package org.esa.sen2agri.services.internal;

import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.entities.ProductTypeInfo;
import org.esa.sen2agri.services.ProductTypesService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

/**
 * Created by cosmin on 1/20/2018.
 */
@Service("productTypesService")
public class ProductTypesServiceImpl implements ProductTypesService {

    @Autowired
    private PersistenceManager persistenceManager;

    @Override
    public List<ProductTypeInfo> list() {
        return persistenceManager.getProductTypes();
    }
}
