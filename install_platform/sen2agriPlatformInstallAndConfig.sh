#!/bin/bash
#set -x #echo on

##
## SCRIPT: INSTALL AND CONFIGURE PLATFORM SEN2AGRI
##
##
## SCRIPT STEPS
##     - INSTALL OTB, SEN2AGRI PROCESSORS AND SEN2AGRI SERVICE
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
: ${INSTAL_CONFIG_FILE:="./config/install_config.conf"}
: ${GPT_CONFIG_FILE:="./config/gpt.vmoptions"}
#-----------------------------------------------------------------------------------------#
: ${SYS_ACC_NAME:="sen2agri-service"}
: ${SLURM_ACC_NAME:="slurm"}
: ${MUNGE_ACC_NAME:="munge"}
#-----------------------------------------------------------------------------------------#
: ${SLURM_CONF_PATH:="/etc/slurm"}
: ${SLURM_CLUSTER_NAME:="sen2agri"}
: ${SLURM_MACHINE_NOCPUS:=$(cat /proc/cpuinfo | grep processor | wc -l)}
: ${SLURM_CONFIG:="slurm.conf"}
: ${SLURM_CONFIG_DB:="slurmdbd.conf"}
: ${SLURM_QOS_LIST:="qosMaccs,qosComposite,qosCropMask,qosCropType,qosPheno,qosLai"}
#----------------SLURM MYSQL DATABASE CREATION---------------------------------------------#
MYSQL_DB_CREATION="create database slurm_acct_db;create user slurm@localhost;
set password for slurm@localhost = password('sen2agri');"
MYSQL_DB_ACCESS_GRANT="grant usage on *.* to slurm;grant all privileges on slurm_acct_db.* to slurm;flush privileges;"
MYSQL_CMD=${MYSQL_DB_CREATION}${MYSQL_DB_ACCESS_GRANT}
#----------------SEN2AGRI POSTGRESQL DATABASE NAME-----------------------------------------#
: ${SEN2AGRI_DATABASE_NAME:="sen2agri"}
#------------------------------------------------------------------------------------------#
declare -r -i -g L1C_PROCESSOR_MACCS=1
declare -r -i -g L1C_PROCESSOR_MAJA=2
#------------------------------------------------------------------------------------------#
function get_install_config_property
{
    grep "^$1=" "${INSTAL_CONFIG_FILE}" | cut -d'=' -f2 | sed -e 's/\r//g'
}

#-----------------------------------------------------------#

function parse_and_update_slurm_conf_file()
{
   ####################################
   ####  copy conf files to /etc/slurm
   ####################################
   cp $(find ./ -name ${SLURM_CONFIG}) ${SLURM_CONF_PATH}
   cp $(find ./ -name ${SLURM_CONFIG_DB}) ${SLURM_CONF_PATH}

   sed -ri "s|CPUs=.+|CPUs=${SLURM_MACHINE_NOCPUS}|g" /etc/slurm/slurm.conf
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
function config_docker()
{
    cd docker
    docker-compose up -d
    cd ..
}
#-----------------------------------------------------------#
function install_and_config_postgresql()
{
   # NB: the container uses `trust`, not `peer` for local connections

   # Install `psql` and client libraries
   yum -y install postgresql12

   #------------DATABASE CREATION------------#
    #DB_NAME=$(head -q -n 1 ./config/db_name.conf 2>/dev/null)
    DB_NAME=$(get_install_config_property "DB_NAME")
    if [ -z "$DB_NAME" ]; then
        DB_NAME="sen2agri"
    fi
    SEN2AGRI_DATABASE_NAME=${DB_NAME}

    if ! [[ "${SEN2AGRI_DATABASE_NAME}" == "sen2agri" ]] ; then
        echo "Using database '${SEN2AGRI_DATABASE_NAME}'"
        sed -i -- "s/-- DataBase Create: sen2agri/-- DataBase Create: ${SEN2AGRI_DATABASE_NAME}/g" ./database/00-database/sen2agri.sql
        sed -i -- "s/CREATE DATABASE sen2agri/CREATE DATABASE ${SEN2AGRI_DATABASE_NAME}/g" ./database/00-database/sen2agri.sql
        sed -i -- "s/-- Privileges: sen2agri/-- Privileges: ${SEN2AGRI_DATABASE_NAME}/g" ./database/09-privileges/privileges.sql
        sed -i -- "s/GRANT ALL PRIVILEGES ON DATABASE sen2agri/GRANT ALL PRIVILEGES ON DATABASE ${SEN2AGRI_DATABASE_NAME}/g" ./database/09-privileges/privileges.sql
    fi

   # first, the database is created. the privileges will be set after all
   # the tables, data and other stuff is created (see down, privileges.sql
   cat "$(find ./ -name "database")/00-database"/sen2agri.sql | su - postgres -c 'psql'

    if ! [[ "${SEN2AGRI_DATABASE_NAME}" == "sen2agri" ]] ; then
        sed -i -re "s|'demmaccs.maccs-launcher',([^,]+),\s+'[^']+'|'demmaccs.maccs-launcher',\1, '${l1c_processor_location}'|" $(find ./ -name "database")/07-data/${SEN2AGRI_DATABASE_NAME}/09.config.sql
        sed -i -re "s|'demmaccs.gips-path',([^,]+),\s+'[^']+'|'demmaccs.gips-path',\1, '${l1c_processor_gipp_destination}'|" $(find ./ -name "database")/07-data/${SEN2AGRI_DATABASE_NAME}/09.config.sql
    else
        sed -i -re "s|'demmaccs.maccs-launcher',([^,]+),\s+'[^']+'|'demmaccs.maccs-launcher',\1, '${l1c_processor_location}'|" $(find ./ -name "database")/07-data/09.config.sql
        sed -i -re "s|'demmaccs.gips-path',([^,]+),\s+'[^']+'|'demmaccs.gips-path',\1, '${l1c_processor_gipp_destination}'|" $(find ./ -name "database")/07-data/09.config.sql
    fi

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
#-----------------------------------------------------------#
function populate_from_scripts()
{
   local curPath=$1
   local customDbPath=${curPath}/${SEN2AGRI_DATABASE_NAME}
   #for each sql scripts found in this folder
   for scriptName in "$curPath"/*.sql
      do
        scriptToExecute=${scriptName}
        if [ -d "$customDbPath" ]; then
            scriptFileName=$(basename -- "$scriptName")

            if [ -f ${customDbPath}/${scriptFileName} ]; then
                scriptToExecute=${customDbPath}/${scriptFileName}
            fi
        fi
         ## perform execution of each sql script
         echo "Executing SQL script: $scriptToExecute"
         cat "$scriptToExecute" | su - postgres -c 'psql '${SEN2AGRI_DATABASE_NAME}''
      done
    # Now check for the custom db folder if there are new scripts, others than in sen2agri. In this case, we must execute them too
    if [ -d "$customDbPath" ]; then
        for scriptName in "$customDbPath"/*.sql
        do
            scriptToExecute=${scriptName}
            scriptFileName=$(basename -- "$scriptName")
            if [[ ! -f ${curPath}/${scriptFileName} ]]; then
                ## perform execution of each sql script
                echo "Executing SQL script: $scriptToExecute"
                cat "$scriptToExecute" | su - postgres -c 'psql '${SEN2AGRI_DATABASE_NAME}''
            fi
        done
    fi
}
#-----------------------------------------------------------#
function install_and_config_webserver()
{
   #install additional packages
   yum -y install php-pgsql

   #install apache
   yum -y install httpd

   #install php
   yum -y install php php-mysql

   sed -i -e 's/; max_input_vars.*/max_input_vars = 10000/' /etc/php.ini
   sed -i -e 's/upload_max_filesize.*/upload_max_filesize = 40M/' /etc/php.ini

   #start service apache
   systemctl start httpd.service

   #enable service apache
   systemctl enable httpd.service

   ##install Sen2Agri Website
   yum -y install ../rpm_binaries/sen2agri-website-*.centos7.x86_64.rpm
}
#-----------------------------------------------------------#
function install_downloaders_demmacs()
{
   ##install prerequisites for Downloaders
   yum -y install wget python-lxml bzip2 python-beautifulsoup4 python-dateutil java-1.8.0-openjdk

   ##install Sen2Agri Downloaders  & Demmacs
   yum -y install ../rpm_binaries/sen2agri-downloaders-demmaccs-*.centos7.x86_64.rpm

   ldconfig

   #reload daemon to update it with new services
   systemctl daemon-reload

   # `systemctl enable --now` doesn't work for timers on CentOS 7
   systemctl enable sen2agri-demmaccs.timer
   systemctl start sen2agri-demmaccs.timer

}
#-----------------------------------------------------------#
function install_RPMs()
{
   ##########################################################
   ####  OTB, SEN2AGRI-PROCESSORS, SEN2AGRI-SERVICES
   ##########################################################

   ##install a couple of packages
   yum -y install gdal-python python-psycopg2 gd

   ##install gdal 2.3 from the local repository
   yum -y install ../rpm_binaries/gdal-local-*.centos7.x86_64.rpm

   # Some GDAL Python tools will pick the wrong GDAL version, but
   # we don't need them
   rm -f /usr/local/bin/gdal_edit.py

   ##install Orfeo ToolBox
   yum -y install ../rpm_binaries/otb-*.rpm

   ##install Sen2Agri Processors
   yum -y install ../rpm_binaries/sen2agri-processors-*.centos7.x86_64.rpm

   ln -s /usr/lib64/libproj.so.0 /usr/lib64/libproj.so
   ldconfig

   ##install Sen2Agri Services
   yum -y install ../rpm_binaries/sen2agri-app-*.centos7.x86_64.rpm

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

#-----------------------------------------------------------#
function install_additional_packages()
{
    if ! [[ "${SEN2AGRI_DATABASE_NAME}" == "sen2agri" ]] ; then
        # Install and config SNAP
        wget http://step.esa.int/downloads/7.0/installers/esa-snap_sentinel_unix_7_0.sh && \
        cp -f esa-snap_sentinel_unix_7_0.sh /tmp/ && \
        chmod +x /tmp/esa-snap_sentinel_unix_7_0.sh && \
        /tmp/esa-snap_sentinel_unix_7_0.sh -q && \
        /opt/snap/bin/snap --nosplash --nogui --modules --update-all
        rm -f ./esa-snap_sentinel_unix_7_0.sh /tmp/esa-snap_sentinel_unix_7_0.sh
        if [ ! -h /usr/local/bin/gpt ]; then sudo ln -s /opt/snap/bin/gpt /usr/local/bin/gpt;fi

        cp -f ${GPT_CONFIG_FILE} /opt/snap/bin/

        # Install R
        yum install -y R-devel libcurl-devel openssl-devel libxml2-devel
        Rscript - <<- EOF
        packages <- c("e1071", "caret", "dplyr", "gsubfn", "ranger", "readr", "smotefamily", "caTools", "tidyverse", "data.table")
        diff <- setdiff(packages, rownames(installed.packages()))
        if (length(diff) > 0) {
            install.packages(diff, repos = c(CRAN = "https://cran.rstudio.com"))
        }
EOF

        # Install Miniconda and the environment for the execution of processors
        SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
        cp "${SCRIPTPATH}/../tools/miniconda/Miniconda3-latest-Linux-x86_64.sh" "/mnt/archive/"
        cp "${SCRIPTPATH}/../tools/miniconda/sen4cap_conda.yml" "/mnt/archive/"

        sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b"
      # sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b -p /mnt/archive/sen4cap_miniconda/miniconda3/"
        sudo -u sen2agri-service bash -c 'echo ". /home/sen2agri-service/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
        sudo su -l sen2agri-service -c bash -c "conda config --set report_errors false"
        if [ -d "/home/sen2agri-service/miniconda3/envs/sen4cap" ] ; then
            sudo rm -fR /home/sen2agri-service/miniconda3/envs/sen4cap
        fi
        sudo su -l sen2agri-service -c bash -c "conda update -n base conda"
        sudo su -l sen2agri-service -c bash -c "conda env create --file=/mnt/archive/sen4cap_conda.yml"
        sudo su -l sen2agri-service -c bash -c "conda info --envs"

        rm "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh"
        rm "/mnt/archive/sen4cap_conda.yml"

    fi
}


function maccs_or_maja()
{
#    while [[ $answer != '1' ]] && [[ $answer != '2' ]]
#    do
#	read -n1 -p "What L1C processor should be used? (1 for MACCS / 2 for MAJA): " -r answer
#	printf "\n"
#	case $answer in
#	    1)
#		echo "MACCS will be used as L1C processor"
#		l1c_processor=$L1C_PROCESSOR_MACCS
# 		;;
#	    2)
#		echo "MAJA will be used as L1C processor"
#		l1c_processor=$L1C_PROCESSOR_MAJA
#		;;
#	    *)
#		echo "Unknown answer"
#		;;
#	esac
#    done
    l1c_processor=$L1C_PROCESSOR_MAJA
    case $l1c_processor in
    $L1C_PROCESSOR_MACCS)
	l1c_processor_name="MACCS"
	l1c_processor_bin="maccs"
	l1c_processor_path="/opt/maccs/core"
	l1c_processor_gipp_destination="/mnt/archive/gipp"
	l1c_processor_gipp_source="../gipp"
	;;
    $L1C_PROCESSOR_MAJA)
	l1c_processor_name="MAJA"
	l1c_processor_bin="maja"
	l1c_processor_path="/opt/maja"
	l1c_processor_gipp_destination="/mnt/archive/gipp_maja"
	l1c_processor_gipp_source="../gipp_maja"
	;;
    *)
	echo "Unknown L1C processor...exit "
	exit
	;;
    esac
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
        read -p "/mnt/upload should be writable by apache. Continue? (y/n) "
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Exiting now"
            exit 1
        fi
    fi

    if ! ls -A $l1c_processor_gipp_destination > /dev/null 2>&1; then
        if [ -d $l1c_processor_gipp_source ]; then
            echo "Copying $l1c_processor_name GIPP files to /mnt/archive"
            cp -rf $l1c_processor_gipp_source /mnt/archive
        else
            echo "Cannot find $l1c_processor_name GIPP files in the distribution, please copy them to $l1c_processor_gipp_destination"
        fi
    fi

    echo "Creating /mnt/archive/reference_data"
    mkdir -p /mnt/archive/reference_data
    echo "Copying reference data"
    if [ -d ../reference_data/ ]; then
        cp -rf ../reference_data/* /mnt/archive/reference_data
    fi
}

function find_l1c_processor()
{
    IFS=$'\n'
    ret=$?
    unset IFS
    echo "ret = $ret"
    l1c_processor_bin_find_out=($(find $l1c_processor_path -name $l1c_processor_bin -type f -executable 2>&-))
    echo "l1c_processor_bin_find_out = $l1c_processor_bin_find_out"
    if [ $ret -eq 0 ] && [ -n "$l1c_processor_bin_find_out" ]; then
        if [ ${#l1c_processor_bin_find_out[@]} -eq 1 ]; then
            l1c_processor_location=${l1c_processor_bin_find_out[0]}
	    echo "${l1c_processor_name} found at ${l1c_processor_location}"
            return 0
        else
	    echo "Multiple ${l1c_processor_name} executables found under ${l1c_processor_path}:"
            printf '%s\n' "${l1c_processor_bin_find_out[@]}"
            return 2
        fi
    else
        echo "Unable to find ${l1c_processor_name} under ${l1c_processor_path}"
        echo "You might want to cancel the setup and install it, or you can use Sen2Cor products if available."
        echo "Hit Ctrl-C now to cancel."
        sleep 20
        return 0
    fi
}

function install_l1c_processor()
{
    yum -y install libxslt gd

    echo "Looking for $l1c_processor_name ..."
    l1c_processor_location=$(type -P $l1c_processor_bin)
    if [ $? -eq 0 ]; then
        echo "$l1c_processor_name found in PATH at $l1c_processor_location"
        status=0
    else
        status=1
    fi

    if [ $status -eq 0 ]; then
        return 0
    fi

    find_l1c_processor
    if [ $? -eq 0 ]; then
        return 0
    fi

    echo "$l1c_processor_name not found, trying to install it"

    found_kit=1
    if [ $l1c_processor -eq $L1C_PROCESSOR_MACCS ]; then
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
    fi
    if [ $l1c_processor -eq $L1C_PROCESSOR_MAJA ]; then
	core_installer=$(find ../maja -name "MAJA*.run" 2>&-)
	if [ $? -ne 0 ] || [ -z "$core_installer" ]; then
	    echo "Unable to find MAJA installer"
	    found_kit=0
	fi
    fi

    if [ $found_kit -eq 1 ]; then
	if [ $l1c_processor -eq $L1C_PROCESSOR_MACCS ]; then
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
	fi
	if [ $l1c_processor -eq $L1C_PROCESSOR_MAJA ]; then
	    echo "Installing MAJA"
	    sh $core_installer || {
		echo "Failed, exiting now"
		exit 1
	    }
	    echo "chmoding"
	    chmod -R a+rx $l1c_processor_path
	fi
	echo "find_l1c_processor"
        find_l1c_processor
        if [ $? -eq 0 ]; then
            return 0
        fi

        echo "Cannot find installed $l1c_processor_name. Did you install it under a different path?"
    fi

    echo "If $l1c_processor_name is already installed, please enter the path to the '$l1c_processor_bin' executable (e.g. /opt/maccs/core/4.7/bin/maccs for MACCS or /otp/maja/bin/maja for MAJA)"
    while :; do
        read -ep "Path to $l1c_processor_bin executable: "
        if [ $? -ne 0 ]; then
            echo "Cancelled, exiting"
            exit 1
        fi

        if [[ -x "$REPLY" ]]; then
            l1c_processor_location=$REPLY
            echo "$l1c_processor_name path seems fine, continuing"
            return 0
        fi

        echo "$REPLY does not seem to point to an executable, please try again or exit with CTRL-C"
    done
}

function updateWebConfigParams()
{
    REST_SERVER_PORT=$(sed -n 's/^server.port =//p' /usr/share/sen2agri/sen2agri-services/config/services.properties | sed -e 's/\r//g')
    # Strip leading space.
    REST_SERVER_PORT="${REST_SERVER_PORT## }"
    # Strip trailing space.
    REST_SERVER_PORT="${REST_SERVER_PORT%% }"
     if [[ !  -z  $REST_SERVER_PORT  ]] ; then
        sed -i -e "s|static \$DEFAULT_REST_SERVICES_URL = \x27http:\/\/localhost:8080|static \$DEFAULT_REST_SERVICES_URL = \x27http:\/\/localhost:$REST_SERVER_PORT|g" /var/www/html/ConfigParams.php
     fi

    DB_NAME=$(get_install_config_property "DB_NAME")
    if [[ ! -z $DB_NAME ]] ; then
        sed -i -e "s|static \$DEFAULT_DB_NAME = \x27sen2agri|static \$DEFAULT_DB_NAME = \x27${DB_NAME}|g" /var/www/html/ConfigParams.php
    fi
}

# Update /etc/sen2agri/sen2agri.conf with the right database
function updateSen2AgriProcessorsParams()
{
    DB_NAME=$(get_install_config_property "DB_NAME")
    if [[ ! -z $DB_NAME ]] ; then
        sed -i -e "s|DatabaseName=sen2agri|DatabaseName=$DB_NAME|g" /etc/sen2agri/sen2agri.conf
    fi
}

function install_sen2agri_services()
{
    SERVICES_ARCHIVE=$(get_install_config_property "SERVICES_ARCHIVE")
    if [ -z "$SERVICES_ARCHIVE" ]; then
    if [ -f ../sen2agri-services/sen2agri-services*.zip ]; then
        zipArchive=$(ls -at ../sen2agri-services/sen2agri-services*.zip| head -n 1)
        fi
    else
        if [ -f "../sen2agri-services/${SERVICES_ARCHIVE}" ]; then
            zipArchive=$(ls -at "../sen2agri-services/${SERVICES_ARCHIVE}" | head -n 1)
        fi
    fi

    if [ -z ${zipArchive} ] ; then
        echo "No sen2agri-services zip archive provided in ../sen2agri-services"
        echo "Exiting now"
        exit 1
    else
        filename="${zipArchive%.*}"

        echo "Extracting into /usr/share/sen2agri/sen2agri-services from archive $zipArchive ..."

        mkdir -p /usr/share/sen2agri/sen2agri-services && unzip ${zipArchive} -d /usr/share/sen2agri/sen2agri-services
        if [ $? -ne 0 ]; then
            echo "Unable to unpack the sen2agri-services into/usr/share/sen2agri/sen2agri-services"
            echo "Exiting now"
            exit 1
        fi
        # convert any possible CRLF into LF
        tr -d '\r' < /usr/share/sen2agri/sen2agri-services/bin/start.sh > /usr/share/sen2agri/sen2agri-services/bin/start.sh.tmp && cp -f /usr/share/sen2agri/sen2agri-services/bin/start.sh.tmp /usr/share/sen2agri/sen2agri-services/bin/start.sh && rm /usr/share/sen2agri/sen2agri-services/bin/start.sh.tmp
        # ensure the execution flag
        chmod a+x /usr/share/sen2agri/sen2agri-services/bin/start.sh

        # update the database name if needed in the sen2agri-services
        DB_NAME=$(get_install_config_property "DB_NAME")
        if [[ ! -z $DB_NAME ]] ; then
            sed -i -e "s/sen2agri?stringtype=unspecified/${DB_NAME}?stringtype=unspecified/" /usr/share/sen2agri/sen2agri-services/config/services.properties
        fi
    fi
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

#use MACCS or MAJA?
maccs_or_maja

check_paths

disable_selinux
disable_firewall

##install EPEL for dependencies, PGDG for the Postgres client libraries and
yum -y install epel-release https://download.postgresql.org/pub/repos/yum/reporpms/EL-7-x86_64/pgdg-redhat-repo-latest.noarch.rpm
yum -y update epel-release pgdg-redhat-repo-latest
yum -y install docker docker-compose

install_sen2agri_services

install_l1c_processor

#-----------------------------------------------------------#
####  OTB, SEN2AGRI, SLURM INSTALL  & CONFIG     ######
#-----------------------------------------------------------#
## install binaries
install_RPMs
updateSen2AgriProcessorsParams

## create system account
create_system_account

## config and start munge
config_and_start_munge_service

## config and start slurm
config_and_start_slurm_service

config_docker

#-----------------------------------------------------------#
####  POSTGRESQL INSTALL & CONFIG AND DATABASE CREATION #####
#-----------------------------------------------------------#
install_and_config_postgresql

#-----------------------------------------------------------#
####  WEBSERVER                 INSTALL   & CONFIG      #####
#-----------------------------------------------------------#
install_and_config_webserver
updateWebConfigParams

#-----------------------------------------------------------#
####  DOWNLOADERS AND DEMMACS  INSTALL                  #####
#-----------------------------------------------------------#
install_downloaders_demmacs

#-----------------------------------------------------------#
####  ADDITIONAL PACKAGES      INSTALL                  #####
#-----------------------------------------------------------#
install_additional_packages

#-----------------------------------------------------------#
####  START ORCHESTRATOR SERVICES                       #####
#-----------------------------------------------------------#
systemctl enable sen2agri-services
systemctl start sen2agri-services
systemctl enable sen2agri-orchestrator
systemctl start sen2agri-orchestrator
systemctl enable sen2agri-scheduler
systemctl start sen2agri-scheduler
systemctl enable sen2agri-http-listener
systemctl start sen2agri-http-listener
systemctl enable sen2agri-monitor-agent
systemctl start sen2agri-monitor-agent

echo "Please edit the USGS and SciHub credentials in the sen2agri-services.properties for the downloaders!"
#echo "Please edit the following files to set up your USGS and SciHub credentials:"
#echo "/usr/share/sen2agri/sen2agri-downloaders/usgs.txt"
#echo "/usr/share/sen2agri/sen2agri-downloaders/apihub.txt"
