#!/bin/sh

#-----------------------------------------------------------#
function install_database()
{
   #------------DATABASE CREATION------------#
    DB_NAME=$(head -q -n 1 ./config/db_name.conf 2>/dev/null)
    if [ -z "$DB_NAME" ]; then
        DB_NAME="sen2agri"
    fi

    if ! [[ "$DB_NAME" == "sen2agri" ]] ; then
        echo "Using database '$DB_NAME'"
        sed -i -- "s/-- DataBase Create: sen2agri/-- DataBase Create: $DB_NAME/g" ./database/00-database/sen2agri.sql
        sed -i -- "s/CREATE DATABASE sen2agri/CREATE DATABASE $DB_NAME/g" ./database/00-database/sen2agri.sql
        sed -i -- "s/-- Privileges: sen2agri/-- Privileges: $DB_NAME/g" ./database/09-privileges/privileges.sql
        sed -i -- "s/GRANT ALL PRIVILEGES ON DATABASE sen2agri/GRANT ALL PRIVILEGES ON DATABASE $DB_NAME/g" ./database/09-privileges/privileges.sql
    fi
   
   # first, the database is created. the privileges will be set after all
   # the tables, data and other stuff is created (see down, privileges.sql
   cat "$(find ./ -name "database")/00-database"/sen2agri.sql | sudo su - postgres -c 'psql'

   sed -i -re "s|'demmaccs.maccs-launcher',([^,]+),\s+'[^']+'|'demmaccs.maccs-launcher',\1, '${maccs_location}'|" $(find ./ -name "database")/07-data/09.config.sql

   #run scripts populating database
   populate_from_scripts "$(find ./ -name "database")/01-extensions"
   populate_from_scripts "$(find ./ -name "database")/02-types"
   populate_from_scripts "$(find ./ -name "database")/03-tables"
   populate_from_scripts "$(find ./ -name "database")/04-views"
   populate_from_scripts "$(find ./ -name "database")/05-functions"
   populate_from_scripts "$(find ./ -name "database")/06-indexes"
   populate_from_scripts "$(find ./ -name "database")/07-data"
   populate_from_scripts "$(find ./ -name "database")/08-keys"
   # granting privileges to sen2agri-service and admin users
   populate_from_scripts "$(find ./ -name "database")/09-privileges"
}

install_database
