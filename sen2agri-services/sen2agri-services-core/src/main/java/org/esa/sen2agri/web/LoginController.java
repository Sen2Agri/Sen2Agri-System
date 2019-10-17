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

package org.esa.sen2agri.web;

import org.esa.sen2agri.db.PersistenceManager;
import org.esa.sen2agri.web.beans.LoginResponse;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.context.request.RequestContextHolder;
import ro.cs.tao.services.commons.ControllerBase;
import ro.cs.tao.services.commons.ResponseStatus;
import ro.cs.tao.services.commons.ServiceResponse;

/**
 * @author Cosmin Cara
 */
@Controller
public class LoginController extends ControllerBase {
    @Autowired
    private PersistenceManager persistenceManager;

    @RequestMapping(value = "/login", method = RequestMethod.POST, produces = "application/json")
    public ResponseEntity<ServiceResponse<?>> authenticate(@RequestParam("user") String user,
                                                           @RequestParam("pwd") String password) {
        int id = persistenceManager.login(user, password);
        if (id == -1) {
            return prepareResult("Invalid credentials", ResponseStatus.FAILED, HttpStatus.UNAUTHORIZED);
        } else {
            final LoginResponse response = new LoginResponse();
            response.setUserId(id);
            response.setSessionToken(RequestContextHolder.currentRequestAttributes().getSessionId());
            return prepareResult(response);
        }
    }
}
