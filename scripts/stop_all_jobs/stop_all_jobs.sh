#!/bin/bash

echo "Truncating scheduler tables: scheduled_task and scheduled_task_status ..."
echo "TRUNCATE scheduled_task" | sudo su - postgres -c 'psql -d sen2agri'
echo "TRUNCATE scheduled_task_status" | sudo su - postgres -c 'psql -d sen2agri'

echo "Truncating job tables: step task event ..."
echo "TRUNCATE step" | sudo su - postgres -c 'psql -d sen2agri'
echo "TRUNCATE task" | sudo su - postgres -c 'psql -d sen2agri'
echo "TRUNCATE event" | sudo su - postgres -c 'psql -d sen2agri'

echo "Stopping slurm jobs ..."
scancel -u sen2agri-service
