#!/bin/bash
#set -x #echo on

##
## SCRIPT: INSTALL AND CONFIGURE PLATFORM SEN2AGRI
##
##
## SCRIPT STEPS
##     - INSTALL OTB, GDAL, SEN2AGRI PROCESSORS AND SEN2AGRI SERVICE
##     - INSTALL SLURM
##     - CONFIGURE SLURM
##         - PARSE AND UPDATE SLURM.CONF FILE AND SLURMDB.CONF FILE
##         - COPY SLURM.CONF AND SLURMDB.CONF INTO FOLDER /ETC/SLURM/
##         - INSTALL MUNGE SERVICE FOR SLURM AND START IT
##         - INSTALL MYSQL(MARIADB), CREATE SLURM DATABASE
##         - START SLURM DAEMONS: SLURMDB, SLURMCTLD, SLURMD AND SLURM
##         - CREATE SLURM ACCOUNT AND USER
##         - CREATE AND CONFIGURE SLURM QOS
##     - INSTALL, CONFIGURE POSTGRESQL AND CREATE DATABASE FOR SEN2AGRI PLATFORM
##     - INSTALL, CONFIGURE APACHE, PHP AND SEN2AGRI WEBSITE ON THE PLATFORM
##     - INSTALL SEN2AGRI DOWNLOADERS AND START SERVICE ON THE PLATFORM
##     - INSTALL SEN2AGRI DEMMACS AND START SERVICE ON THE PLATFORM
################################################################################################
## SCRIPT USAGE:
##
## open a terminal go into delivery install_script folder:
## cd /path/to/install_script
## sudo ./sen2agriPlatormInstallAndConfig.sh
################################################################################################
: ${SYS_ACC_NAME:="sen2agri-service"}
: ${SLURM_ACC_NAME:="slurm"}
: ${MUNGE_ACC_NAME:="munge"}
#-----------------------------------------------------------------------------------------#
: ${SLURM_MACHINE_HOSTNAME:="$(sed -e 's/[[:space:]]*$//' <<<$(hostname -s))"}
: ${SLURM_CONF_PATH:="/etc/slurm"}
: ${SLURM_USER_NAME:="slurm"}
: ${SLURM_USER_PASS:="sen2agri"}
: ${SLURM_CLUSTER_NAME:="sen2agri"}
: ${SLURM_MACHINE_NOCPUS:=$(cat /proc/cpuinfo | grep processor | wc -l)}
: ${SLURM_CONFIG:="slurm.conf"}
: ${SLURM_CONFIG_DB:="slurmdbd.conf"}
: ${SLURM_PARTITION_NAME_DEF:="sen2agri"}
: ${SLURM_PARTITION_NAME_HI:="sen2agriHi"}
: ${SLURM_PARTITION_PRIORITY_HI:="2"}
: ${SLURM_QOS_LIST:="qosMaccs,qosComposite,qosCropMask,qosCropType,qosPheno,qosLai"}
#----------------SLURM MYSQL DATABASE CREATION---------------------------------------------#
MYSQL_DB_CREATION="create database slurm_acct_db;create user slurm@localhost;
set password for slurm@localhost = password('sen2agri');"
MYSQL_DB_ACCESS_GRANT="grant usage on *.* to slurm;grant all privileges on slurm_acct_db.* to slurm;flush privileges;"
MYSQL_CMD=${MYSQL_DB_CREATION}${MYSQL_DB_ACCESS_GRANT}
#----------------SEN2AGRI POSTGRESQL DATABASE NAME-----------------------------------------#
: ${SEN2AGRI_DATABASE_NAME:="sen2agri"}
#------------------------------------------------------------------------------------------#
function parse_and_update_slurm_conf_file()
{
   ####################################
   ####  slurm.conf
   ####################################
   ##update field ControlMachine
   sed -ri "s|ControlMachine=.+|ControlMachine=${SLURM_MACHINE_HOSTNAME}|g" $(find ./ -name ${SLURM_CONFIG})

   ##update COMPUTE NODES INFO
   sed -ri "s|NodeName=.+|NodeName=${SLURM_MACHINE_HOSTNAME} CPUs=${SLURM_MACHINE_NOCPUS} State=UNKNOWN|g" $(find ./ -name ${SLURM_CONFIG})

   ##update field ClusterName
   sed -ri "s|ClusterName=.+|ClusterName=${SLURM_CLUSTER_NAME}|g" $(find ./ -name ${SLURM_CONFIG})

   ##update PARTITION CONFIGURATION INFO
   ## delete existing PARTITIONS NAMES
   sed -i "/PartitionName/d" $(find ./ -name ${SLURM_CONFIG})

   ##create new PARTITION - default one
   sed -i "\$aPartitionName=${SLURM_PARTITION_NAME_DEF} Nodes=${SLURM_MACHINE_HOSTNAME} Default=YES MaxTime=INFINITE State=UP Shared=FORCE:1" $(find ./ -name ${SLURM_CONFIG})

   ##create new PARTITION - hi prio one
   sed -i "\$aPartitionName=${SLURM_PARTITION_NAME_HI} Nodes=${SLURM_MACHINE_HOSTNAME} Default=NO Priority=${SLURM_PARTITION_PRIORITY_HI} MaxTime=INFINITE State=UP" $(find ./ -name ${SLURM_CONFIG})

   ####################################
   ####  slurmdbd.conf
   ####################################
   ##update DbdAddr INFO
   sed -ri "s|DbdAddr=.+|DbdAddr=${SLURM_MACHINE_HOSTNAME}|g" $(find ./ -name ${SLURM_CONFIG_DB})

   ##update DbdHost INFO
   sed -ri "s|DbdHost=.+|DbdHost=${SLURM_MACHINE_HOSTNAME}|g" $(find ./ -name ${SLURM_CONFIG_DB})

   ##update StoragePass INFO
   sed -ri "s|StoragePass=.+|StoragePass=${SLURM_USER_PASS}|g" $(find ./ -name ${SLURM_CONFIG_DB})

   ####################################
   ####  copy conf files to /etc/slurm
   ####################################
   cp $(find ./ -name ${SLURM_CONFIG}) ${SLURM_CONF_PATH}
   cp $(find ./ -name ${SLURM_CONFIG_DB}) ${SLURM_CONF_PATH}
}
#-----------------------------------------------------------#
function create_slurm_data_base()
{
   ##install expect
   yum -y install expect expectk

   ##install mysql (mariadb)
   yum -y install mariadb-server mariadb

   ##start mysql (mariadb)
   systemctl start mariadb

   ##enable mysql (mariadb) to start at boot
   systemctl enable mariadb

   ##get status of mysql service
   echo "MYSQL SERVICE: $(systemctl status mariadb | grep "Active")"

   ##install secure mysql
   SECURE_MYSQL=$(expect -c "
      set timeout 10
      spawn mysql_secure_installation

      expect \"Enter current password for root (enter for none):\"
      send \"\r\"

      expect \"Change the root password?\"
      send \"n\r\"

      expect \"Remove anonymous users?\"
      send \"y\r\"

      expect \"Disallow root login remotely?\"
      send \"y\r\"

      expect \"Remove test database and access to it?\"
      send \"y\r\"

      expect \"Reload privilege tables now?\"
      send \"y\r\"
      expect eof
   ")

   echo "$SECURE_MYSQL"

   ##create database for slurm service
   DB_MYSQL=$(expect -c "
      set timeout 5
      spawn mysql -u root -p -e \"${MYSQL_CMD}\"
      expect \"Enter password:\"
      send \"\r\"
      expect eof
   ")

   echo "$DB_MYSQL"
}
#-----------------------------------------------------------#
function config_and_start_slurm_service()
{
   #create slurm account for service install
   adduser -m ${SLURM_ACC_NAME}

   ####################################
   ####  process SLURM .conf files
   ####################################
   parse_and_update_slurm_conf_file

   ####################################
   ####  SLURM database create and config
   ####################################
   create_slurm_data_base

   ####################################
   ####  SLURM Daemons start
   ####################################
   ##start slurmdbd (slurmdbd)
   systemctl start slurmdbd

   ##enable slurmdbd (slurmdbd)  to start at boot
   systemctl enable slurmdbd

   ##get status of slurmdbd service
   echo "SLURM DB SERVICE: $(systemctl status slurmdbd | grep "Active")"

   ##create the cluster in the accounting system
   CLUSTER_CREATE=$(expect -c "
      set timeout 5
      spawn sacctmgr add cluster \"${SLURM_CLUSTER_NAME}\"
      expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
      send \"y\r\"
      expect eof
   ")
   echo "$CLUSTER_CREATE"

   ##create SLURM spool and log directories and set permissions accordingly
   mkdir /var/spool/slurm
   chown -R slurm:slurm /var/spool/slurm
   mkdir /var/log/slurm
   chown -R slurm:slurm /var/log/slurm

   ##start slurm controller daemon slurmctld (slurmctld)
   systemctl start slurmctld

   ##enable slurm controller daemon slurmctld to start at boot
   systemctl enable slurmctld

   ##get status of slurm controller daemon  slurmctld service
   echo "SLURM CTL SERVICE: $(systemctl status slurmctld | grep "Active")"

   ##start slurm node daemon slurmd (slurmd)
   systemctl start slurmd

   ##enable slurm node daemon slurmd to start at boot
   systemctl enable slurmd

   ##get status of slurm node daemon slurmd service
   echo "SLURM NODE SERVICE: $(systemctl status slurmd | grep "Active")"

   ##start slurm service (slurm)
   systemctl start slurm

   ##enable slurm service to start at boot
   systemctl enable slurm

   ##get status of slurm service service
   echo "SLURM SERVICE: $(systemctl status slurm | grep "Active")"

   ####################################
   ####  SLURM post config
   ####################################
   ## create account in slurm
   create_slurm_account

   ## create QOS in slurm
   create_and_config_slurm_qos
}
#-----------------------------------------------------------#

function config_and_start_munge_service()
{
   #create munge account for service install
   adduser -m ${MUNGE_ACC_NAME}

   ##secure installation  - set permissions on munge folders
   chmod 755 /etc/munge
   chmod 755 /var/lib/munge/
   chmod 755 /var/log/munge/
   chmod 755 /var/run/munge/

   ##generate MUNGE key
   dd if=/dev/urandom bs=1 count=1024 > /etc/munge/munge.key
   chown munge:munge /etc/munge/munge.key
   chmod 400 /etc/munge/munge.key

   ##enable munge Daemon to start on boot time
   systemctl enable munge

   ##start munge Daemon
   systemctl start munge

   ##get status of munge daemon
   echo "MUNGE SERVICE: $(systemctl status munge | grep "Active")"

}
#-----------------------------------------------------------#
function create_system_account()
{
   #create system account for running services
   adduser -m ${SYS_ACC_NAME}
}
#-----------------------------------------------------------#
function create_slurm_account()
{
   #create SLURM account for running application
   SLURM_ACC_CREATE=$(expect -c "
      set timeout 5
      spawn sacctmgr add account \"${SYS_ACC_NAME}\"
      expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
      send \"y\r\"
      expect eof
   ")
   echo "$SLURM_ACC_CREATE"

   #create user associated to the account
   SLURM_USER_CREATE=$(expect -c "
      set timeout 5
      spawn sacctmgr add user \"${SYS_ACC_NAME}\" Account=\"${SYS_ACC_NAME}\"
      expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
      send \"y\r\"
      expect eof
   ")
   echo "$SLURM_USER_CREATE"

   #add privileges to the user
   SLURM_USER_LEVEL=$(expect -c "
      set timeout 5
      spawn sacctmgr modify user \"${SYS_ACC_NAME}\" set adminlevel=Admin
      expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
      send \"y\r\"
      expect eof
   ")
   echo "$SLURM_USER_LEVEL"
}

#-----------------------------------------------------------#
function create_and_config_slurm_qos()
{
   #extract each configured QOS from SLURM_QOS_LIST
   IFS=',' read -ra ADDR <<< "${SLURM_QOS_LIST}"

   #for each qos defined in configuration
   for qosName in "${ADDR[@]}"; do
      #add qos to slurm
      SLURM_ADD_QOS=$(expect -c "
         set timeout 5
         spawn sacctmgr add qos  \"${qosName}\"
         expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
         send \"y\r\"
         expect eof
      ")
      echo "$SLURM_ADD_QOS"

      #set qos number of jobs able to run at any given time
      SLURM_JOBS_PER_QOS=$(expect -c "
         set timeout 5
         spawn sacctmgr modify qos "${qosName}" set GrpJobs=1
         expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
         send \"y\r\"
         expect eof
      ")
      echo "$SLURM_JOBS_PER_QOS"

      #add already created qos to user , and another qos if that qos already exists
      SLURM_ADD_QOS_TO_ACC=$(expect -c "
         set timeout 5
         spawn sacctmgr modify user "${SYS_ACC_NAME}" set qos+="${qosName}"
         expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
         send \"y\r\"
         expect eof
      ")
      echo "$SLURM_ADD_QOS_TO_ACC"
   done

   #add implicit SLURM QOS "normal" to user
   SLURM_ADD_QOS_TO_ACC=$(expect -c "
      set timeout 5
      spawn sacctmgr modify user "${SYS_ACC_NAME}" set qos+=normal
      expect \"Would you like to commit changes? (You have 30 seconds to decide)\"
      send \"y\r\"
      expect eof
   ")
   echo "$SLURM_ADD_QOS_TO_ACC"

   #show current configuration for SLURM
   echo "CLUSTER,USERS,QOS INFO:"
   sacctmgr show assoc format=cluster,user,qos

   echo "QOS INFO:"
   sacctmgr list qos

   echo "Partition INFO:"
   scontrol show partition

   echo "Nodes INFO:"
   scontrol show node
}
#-----------------------------------------------------------#
function install_and_config_postgresql()
{
   #------------INSTALL AND START------------#
   #install PostgreSQL
   yum -y localinstall http://yum.postgresql.org/9.4/redhat/rhel-7.2-x86_64/pgdg-centos94-9.4-2.noarch.rpm
   yum -y install postgresql94-server

   #initialize the database in PGDATA
   /usr/pgsql-9.4/bin/postgresql94-setup initdb

   #install pgcrypto in PostgreSQL
   yum -y install postgresql94-contrib

   #install PostGIS
   yum -y install postgis2_94

   #start service Postgresql and enable to start at boot
   systemctl enable postgresql-9.4.service
   systemctl start postgresql-9.4.service

   ##get status of postgresql-9.4 daemon
   echo "POSTGRESQL SERVICE: $(systemctl status postgresql-9.4 | grep "Active")"

   #------------DATABASE CREATION------------#
   # first, the database is created. the privileges will be set after all
   # the tables, data and other stuff is created (see down, privileges.sql
   cat "$(find ./ -name "database")/00-database"/sen2agri.sql | sudo su - postgres -c 'psql'

   local _inet_addr="$(ip -4 a | grep "inet " | grep -v " lo" | tr -s ' ' | cut "-d " -s -f3 | cut -d/ -f1 | head -n1)"
   sed -i -re "/'executor.listen-ip'/ { /'/ s/'[0-9.]+'/'${_inet_addr}'/ }" $(find ./ -name "database")/07-data/09.config.sql
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

   #-------------- pg_hba.conf -----------------------#
   ####  copy conf file to /var/lib/pgsql/9.4/data/pg_hba.conf
   cp -f $(find ./ -name "pg_hba.conf") /var/lib/pgsql/9.4/data/

   #restart service Postgresql
   systemctl restart postgresql-9.4.service
}
#-----------------------------------------------------------#
function populate_from_scripts()
{
   local curPath=$1
   #for each sql scripts found in this folder
   for scriptName in "$curPath"/*.sql
      do
         ## perform execution of each sql script
         echo "Executing SQL script: $scriptName"
         cat "$scriptName" | sudo su - postgres -c 'psql '${SEN2AGRI_DATABASE_NAME}''
      done
}
#-----------------------------------------------------------#
function install_and_config_webserver()
{
   #install additional packages
   yum -y install php-pgsql

   #install apache
   yum -y install httpd

   #start service apache
   systemctl start httpd.service

   #enable service apache
   systemctl enable httpd.service

   #install php
   yum -y install php php-mysql

   #restart service apache
   systemctl restart httpd.service

   ##install Sen2Agri Website
   yum -y install ../rpm_binaries/sen2agri-website-1.0.centos7.x86_64.rpm

   #get machine IP
   local _inet_addr="$(ip -4 a | grep "inet " | grep -v " lo" | tr -s ' ' | cut "-d " -s -f3 | cut -d/ -f1 | head -n1)"

   ##update file /var/www/html/ConfigParams.php
   ##replace "sen2agri-dev" with machine ip "_inet_addr "  into file /var/www/html/ConfigParams.php
   local _host_name="$(hostname -s)"
   sed -i "s/sen2agri-dev/$_host_name/" /var/www/html/ConfigParams.php
   #sed -i "s/sen2agri-dev/$_inet_addr/" /var/www/html/ConfigParams.php

}
#-----------------------------------------------------------#
function install_downloaders_demmacs()
{
   ##install wget , python-lxml and bzip prerequisites for Downloaders
   yum -y install wget python-lxml bzip2

   ##install java prerequisites for Downloaders
   yum -y install java-1.8.0-openjdk

   ##install Sen2Agri Downloaders  & Demmacs
   yum -y install ../rpm_binaries/sen2agri-downloaders-demmaccs-1.0.centos7.x86_64.rpm

   echo /usr/local/lib | sudo tee /etc/ld.so.conf.d/local.conf
   ldconfig

   #reload daemon to update it with new services
   systemctl daemon-reload

   #start imediately the services for downloaders and demmacs
   systemctl enable --now sen2agri-landsat-downloader.timer
   systemctl start sen2agri-landsat-downloader.service
   systemctl enable --now sen2agri-sentinel-downloader.timer
   systemctl start sen2agri-sentinel-downloader.service
   #the demmaccs service may be started whenever it's time will come
   #starting it at the same time with downloaders, will do nothing 'cause it
   #will not find any downloaded product
   systemctl enable --now sen2agri-demmaccs.timer

}
#-----------------------------------------------------------#
function install_RPMs()
{
   ##########################################################
   ####  OTB, GDAL, SEN2AGRI-PROCESSORS, SEN2AGRI-SERVICES
   ##########################################################

   ##install a couple of packages
   yum -y install cifs-utils gdal-python python-psycopg2 gd

   ##install Orfeo ToolBox
   yum -y install ../rpm_binaries/otb-5.0.centos7.x86_64.rpm

   ##install GDAL library
   yum -y install ../rpm_binaries/gdal-local-2.0.1.centos7.x86_64.rpm

   ##install Sen2Agri Processors
   yum -y install ../rpm_binaries/sen2agri-processors-0.8.centos7.x86_64.rpm

   echo /usr/local/lib > /etc/ld.so.conf.d/local.conf
   ln -s /usr/lib64/libproj.so.0 /usr/lib64/libproj.so
   ldconfig

   ##install Sen2Agri Services
   yum -y install ../rpm_binaries/sen2agri-app.centos7.x86_64.rpm

   ##########################################################
   ####  SLURM
   ##########################################################

   yum -y install ../rpm_binaries/slurm/slurm-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-devel-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-munge-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-perlapi-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-pam_slurm-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-plugins-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-sjobexit-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-sjstat-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-slurmdbd-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-slurmdb-direct-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-sql-15.08.7-1.el7.centos.x86_64.rpm \
../rpm_binaries/slurm/slurm-torque-15.08.7-1.el7.centos.x86_64.rpm
}

function check_paths()
{
    echo "Checking paths..."

    if [ ! -d /mnt/archive ]; then
        echo "Please create /mnt/archive with mode 777."
        echo "Actually only the sen2agri-service and apache users require access to the directory, but the installer does not support that."
        echo "Exiting now"
        exit 1
    fi

    out=($(stat -c "%a %U" /mnt/archive))
    if [ "${out[0]}" != "777" ] && [ "${out[1]}" != "sen2agri-service" ]; then
        read -p "/mnt/archive should be writable by sen2agri-service. Continue? (y/n) "
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Exiting now"
            exit 1
        fi
    fi

    if ! ls -A /mnt/archive/srtm > /dev/null 2>&1; then
        if [ -f ../srtm.zip ]; then
            mkdir -p /mnt/archive/srtm && unzip ../srtm.zip -d /mnt/archive/srtm
            if [ $? -ne 0 ]; then
                echo "Unable to unpack the SRTM dataset into /mnt/archive/srtm"
                echo "Exiting now"
                exit 1
            fi
        else
            echo "Please unpack the SRTM dataset into /mnt/archive/srtm"
            echo "Exiting now"
            exit 1
        fi
    fi

    if ! ls -A /mnt/archive/swbd > /dev/null 2>&1; then
        if [ -f ../swbd.zip ]; then
            mkdir -p /mnt/archive/swbd && unzip ../swbd.zip -d /mnt/archive/swbd
            if [ $? -ne 0 ]; then
                echo "Unable to unpack the SWBD dataset into /mnt/archive/swbd"
                echo "Exiting now"
                exit 1
            fi
        else
            echo "Please unpack the SWBD dataset into /mnt/archive/swbd"
            echo "Exiting now"
            exit 1
        fi
    fi

    if [ ! -d /mnt/upload ]; then
        echo "Please create /mnt/upload making sure it's writable by the apache user and readable by sen2agri-service."
        echo "Exiting now"
        exit 1
    fi

    out=($(stat -c "%a %U" /mnt/upload))
    if [ "${out[0]}" != "777" ] && [ "${out[1]}" != "apache" ]; then
        read -p "/mnt/upload should be writable by sen2agri-service. Continue? (y/n) "
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Exiting now"
            exit 1
        fi
    fi

    if ! ls -A /mnt/archive/gipp > /dev/null 2>&1; then
        if [ -d ../gipp ]; then
            echo "Copying MACCS GIPP files to /mnt/archive"
            cp -rf ../gipp /mnt/archive
        else
            echo "Cannot find MACCS GIPP files in the distribution, please copy them to /mnt/archive/gipp"
        fi
    fi
}

function find_maccs()
{
    IFS=$'\n'
    out=($(find /opt/maccs/core -name maccs -type f -executable 2>&-))
    ret=$?
    unset IFS
    if [ $ret -eq 0 ] && [ -n "$out" ]; then
        if [ ${#out[@]} -eq 1 ]; then
            maccs_location=${out[0]}
            echo "MACCS found at ${maccs_location}"
            return 0
        else
            echo "Multiple MACCS executables found under /opt/maccs/core:"
            printf '%s\n' "${out[@]}"
            return 2
        fi
    else
        echo "Unable to find MACCS under /opt/maccs/core"
        return 1
    fi
}

function install_maccs()
{
    yum -y install libxslt gd

    echo "Looking for MACCS..."
    maccs_location=$(type -P maccs)
    if [ $? -eq 0 ]; then
        echo "MACCS found in PATH at $maccs_location"
        status=0
    else
        status=1
    fi

    if [ $status -eq 0 ]; then
        return 0
    fi

    find_maccs
    if [ $? -eq 0 ]; then
        return 0
    fi

    echo "MACCS not found, trying to install it"

    found_kit=1
    cots_installer=$(find ../maccs/cots -name install-maccs-cots.sh 2>&-)
    if [ $? -ne 0 ] || [ -z "$cots_installer" ]; then
        echo "Unable to find MACCS COTS installer"
        found_kit=0
    fi
    if [ $found_kit -eq 1 ]; then
        core_installer=$(find ../maccs/core -name "install-maccs-*.sh" 2>&-)
        if [ $? -ne 0 ] || [ -z "$core_installer" ]; then
            echo "Unable to find MACCS installer"
            found_kit=0
        fi
    fi

    if [ $found_kit -eq 1 ]; then
        yum -y install redhat-lsb-core

        echo "Installing MACCS COTS"
        sh $cots_installer || {
            echo "Failed, exiting now"
            exit 1
        }
        echo "Installing MACCS"
        sh $core_installer || {
            echo "Failed, exiting now"
            exit 1
        }

        find_maccs
        if [ $? -eq 0 ]; then
            return 0
        fi

        echo "Cannot find installed MACCS. Did you install it under a different path?"
    fi

    echo "If MACCS is already installed, please enter the path to the 'maccs' executable (e.g. /opt/maccs/core/4.7/bin/maccs)"
    while :; do
        read -ep "Path to maccs executable: "
        if [ $? -ne 0 ]; then
            echo "Cancelled, exiting"
            exit 1
        fi

        if [[ -x "$REPLY" ]]; then
            maccs_location=$REPLY
            echo "MACCS path seems fine, continuing"
            return 0
        fi

        echo "$REPLY does not seem to point to an executable, please try again or exit with CTRL-C"
    done
}

function disable_selinux()
{
    echo "Disabling SELinux"
    echo "The Sen2Agri system is not inherently incompatible with SELinux, but relabelling the file system paths is not implemented yet in the installer."
    setenforce 0
    sed -i -e 's/SELINUX=enforcing/SELINUX=permissive/' /etc/selinux/config
}

# This is needed for SLURM because it uses dynamically-allocated ports
# The other services could do with a couple of rules
function disable_firewall()
{
    echo "Disabling the firewall"
    firewall-cmd --set-default-zone=trusted
    firewall-cmd --reload
}

###########################################################
##### MAIN                                              ###
###########################################################

if [ $EUID -ne 0 ]; then
    echo "This setup script must be run as root. Exiting now."
    exit 1
fi

check_paths

disable_selinux
disable_firewall

##install EPEL for packages dependencies installation
yum -y install epel-release

install_maccs

#-----------------------------------------------------------#
####  OTB, GDAL, SEN2AGRI, SLURM INSTALL  & CONFIG     ######
#-----------------------------------------------------------#
## install binaries
install_RPMs

## create system account
create_system_account

## config and start munge
config_and_start_munge_service

## config and start slurm
config_and_start_slurm_service

#-----------------------------------------------------------#
####  POSTGRESQL INSTALL & CONFIG AND DATABASE CREATION #####
#-----------------------------------------------------------#
install_and_config_postgresql

#-----------------------------------------------------------#
####  WEBSERVER                 INSTALL   & CONFIG      #####
#-----------------------------------------------------------#
install_and_config_webserver

#-----------------------------------------------------------#
####  DOWNLOADERS AND DEMMACS  INSTALL                  #####
#-----------------------------------------------------------#
install_downloaders_demmacs

#-----------------------------------------------------------#
####  START ORCHESTRATOR SERVICES                       #####
#-----------------------------------------------------------#
systemctl enable --now sen2agri-orchestrator
systemctl enable --now sen2agri-scheduler
systemctl enable --now sen2agri-http-listener

echo "Please edit the following files to set up your USGS and SciHub credentials:"
echo "/usr/share/sen2agri/sen2agri-downloaders/usgs.txt"
echo "/usr/share/sen2agri/sen2agri-downloaders/apihub.txt"
