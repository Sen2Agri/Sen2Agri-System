#!/bin/bash

echo "Truncating tables: step task event ..."
echo "TRUNCATE step" | sudo su - postgres -c 'psql -d sen2agri'
echo "TRUNCATE task" | sudo su - postgres -c 'psql -d sen2agri'
echo "TRUNCATE event" | sudo su - postgres -c 'psql -d sen2agri'

echo "Stopping slurm jobs ..."
scancel -u sen2agri-service
