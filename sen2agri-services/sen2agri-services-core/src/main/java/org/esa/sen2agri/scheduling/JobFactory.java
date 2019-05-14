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
package org.esa.sen2agri.scheduling;

import org.quartz.Job;
import org.quartz.SchedulerContext;
import org.quartz.spi.TriggerFiredBundle;
import org.springframework.beans.BeanWrapper;
import org.springframework.beans.BeansException;
import org.springframework.beans.MutablePropertyValues;
import org.springframework.beans.PropertyAccessorFactory;
import org.springframework.beans.factory.config.AutowireCapableBeanFactory;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.scheduling.quartz.SpringBeanJobFactory;
import org.springframework.stereotype.Component;

/**
 * @author Cosmin Cara
 */
@Component
public class JobFactory extends SpringBeanJobFactory implements ApplicationContextAware {
    private ApplicationContext context;
    private SchedulerContext schedulerContext;
    private transient AutowireCapableBeanFactory beanFactory;

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.beanFactory = applicationContext.getAutowireCapableBeanFactory();
        this.context = applicationContext;
    }

    @Override
    protected Object createJobInstance(TriggerFiredBundle bundle) throws Exception {
        Job job = context.getBean(bundle.getJobDetail().getJobClass());
        BeanWrapper bw = PropertyAccessorFactory.forBeanPropertyAccess(job);
        MutablePropertyValues pvs = new MutablePropertyValues();
        pvs.addPropertyValues(bundle.getJobDetail().getJobDataMap());
        pvs.addPropertyValues(bundle.getTrigger().getJobDataMap());
        if (this.schedulerContext != null) {
            pvs.addPropertyValues(this.schedulerContext);
        }
        bw.setPropertyValues(pvs, true);
        return job;
    }

    @Override
    public void setSchedulerContext(SchedulerContext schedulerContext) {
        this.schedulerContext = schedulerContext;
        super.setSchedulerContext(schedulerContext);
    }
}
