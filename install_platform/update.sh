#!/bin/sh

INSTAL_CONFIG_FILE="./config/install_config.conf"
HAS_S2AGRI_SERVICES=false

function get_install_config_property
{
    grep "^$1=" "${INSTAL_CONFIG_FILE}" | cut -d'=' -f2 | sed -e 's/\r//g'
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

    # Check if directory does not exists or is empty
    if [ ! -d "${TARGET_SERVICES_DIR}" ] || [ ! "$(ls -A ${TARGET_SERVICES_DIR})" ] ; then
        if [ -f ../sen2agri-services/${SERVICES_ARCHIVE} ]; then
            echo "Extracting into ${TARGET_SERVICES_DIR} from archive $zipArchive ..."

            mkdir -p ${TARGET_SERVICES_DIR} && unzip ${zipArchive} -d ${TARGET_SERVICES_DIR}
            if [ $? -ne 0 ]; then
                echo "Unable to unpack the sen2agri-services into ${TARGET_SERVICES_DIR}"
                echo "Exiting now"
                exit 1
            fi
            # convert any possible CRLF into LF
            tr -d '\r' < ${TARGET_SERVICES_DIR}/bin/start.sh > ${TARGET_SERVICES_DIR}/bin/start.sh.tmp && cp -f ${TARGET_SERVICES_DIR}/bin/start.sh.tmp ${TARGET_SERVICES_DIR}/bin/start.sh && rm ${TARGET_SERVICES_DIR}/bin/start.sh.tmp
            # ensure the execution flag
            chmod a+x ${TARGET_SERVICES_DIR}/bin/start.sh
        else
            echo "No sen2agri-services zip archive provided in ../sen2agri-services"
            echo "Exiting now"
            exit 1
        fi
    else
        echo "sen2agri-services already exist in ${TARGET_SERVICES_DIR}"
        if [ -d "${TARGET_SERVICES_DIR}/bin" ] && [ -d "${TARGET_SERVICES_DIR}/config" ] ; then
            if [ -f ../sen2agri-services/${SERVICES_ARCHIVE} ]; then
                echo "Updating ${TARGET_SERVICES_DIR}/lib folder ..."
                mkdir -p ${TARGET_SERVICES_DIR}/lib && rm -f ${TARGET_SERVICES_DIR}/lib/*.jar && unzip -o ${zipArchive} 'lib/*' -d ${TARGET_SERVICES_DIR}
                echo "Updating ${TARGET_SERVICES_DIR}/modules folder ..."
                mkdir -p ${TARGET_SERVICES_DIR}/modules && rm -f ${TARGET_SERVICES_DIR}/modules/*.jar && unzip -o ${zipArchive} 'modules/*' -d ${TARGET_SERVICES_DIR}

                echo "Updating ${TARGET_SERVICES_DIR}/static folder ..."
                mkdir -p ${TARGET_SERVICES_DIR}/static && rm -fR ${TARGET_SERVICES_DIR}/static/* && unzip -o ${zipArchive} 'static/*' -d ${TARGET_SERVICES_DIR}

                mkdir -p ${TARGET_SERVICES_DIR}/scripts

                if [ -f ${TARGET_SERVICES_DIR}/config/sen2agri-services.properties ] ; then
                    mv ${TARGET_SERVICES_DIR}/config/sen2agri-services.properties ${TARGET_SERVICES_DIR}/config/services.properties
                fi
                if [ -f ${TARGET_SERVICES_DIR}/config/application.properties ] ; then
                    cp -f ${TARGET_SERVICES_DIR}/config/application.properties ${TARGET_SERVICES_DIR}/config/application.properties.bkp
                fi
                # update the application.properties file even if some user changes might be lost
                unzip -o ${zipArchive} 'config/application.properties' -d ${TARGET_SERVICES_DIR}/
            else
                echo "No archive sen2agri-services-YYY.zip was found in the installation package. sen2agri-services will not be updated!!!"
            fi
        else
            echo "ERROR: no bin or config folder were found in the folder ${TARGET_SERVICES_DIR}/. No update will be made!!!"
        fi
        HAS_S2AGRI_SERVICES=true
    fi
}

SCIHUB_USER=""
SCIHUB_PASS=""
USGS_USER=""
USGS_PASS=""
function saveOldDownloadCredentials()
{
    if [ -f /usr/share/sen2agri/sen2agri-downloaders/apihub.txt ]; then
        apihubLine=($(head -n 1 /usr/share/sen2agri/sen2agri-downloaders/apihub.txt))
        SCIHUB_USER=${apihubLine[0]}
        SCIHUB_PASS=${apihubLine[1]}
    fi
    if [ -f /usr/share/sen2agri/sen2agri-downloaders/usgs.txt ]; then
        usgsLine=($(head -n 1 /usr/share/sen2agri/sen2agri-downloaders/usgs.txt))
        USGS_USER=${usgsLine[0]}
        USGS_PASS=${usgsLine[1]}
    fi
}

function updateDownloadCredentials()
{
    if [ "$HAS_S2AGRI_SERVICES" = false ] ; then
        sed -i '/SciHubDataSource.username=/c\SciHubDataSource.username='"$SCIHUB_USER" ${TARGET_SERVICES_DIR}/config/services.properties
        sed -i '/SciHubDataSource.password=/c\SciHubDataSource.password='"$SCIHUB_PASS" ${TARGET_SERVICES_DIR}/config/services.properties
        sed -i '/USGSDataSource.username=/c\USGSDataSource.username='"$USGS_USER" ${TARGET_SERVICES_DIR}/config/services.properties
        sed -i '/USGSDataSource.password=/c\USGSDataSource.password='"$USGS_PASS" ${TARGET_SERVICES_DIR}/config/services.properties
    fi
}

function enableSciHubDwnDS()
{
    echo "Disabling Amazon datasource ..."
    sed -i '/SciHubDataSource.Sentinel2.scope=1/c\SciHubDataSource.Sentinel2.scope=3' ${TARGET_SERVICES_DIR}/config/services.properties
    sed -i '/AWSDataSource.Sentinel2.enabled=true/c\AWSDataSource.Sentinel2.enabled=false' ${TARGET_SERVICES_DIR}/config/services.properties

    sed -i 's/AWSDataSource.Sentinel2.local_archive_path=/SciHubDataSource.Sentinel2.local_archive_path=/g' ${TARGET_SERVICES_DIR}/config/services.properties
    sed -i 's/AWSDataSource.Sentinel2.fetch_mode=/SciHubDataSource.Sentinel2.fetch_mode=/g' ${TARGET_SERVICES_DIR}/config/services.properties

    sudo -u postgres psql $DB_NAME -c "update datasource set scope = 3 where satellite_id = 1 and name = 'Scientific Data Hub';"
    sudo -u postgres psql $DB_NAME -c "update datasource set enabled = 'false' where satellite_id = 1 and name = 'Amazon Web Services';"
    echo "Disabling Amazon datasource ... Done!"

#    sudo -u postgres psql sen2agri -c "update datasource set local_root = (select local_root from datasource where satellite_id = 1 and name = 'Amazon Web Services') where satellite_id = 1 and name = 'Scientific Data Hub';"
#    sudo -u postgres psql sen2agri -c "update datasource set fetch_mode = (select fetch_mode from datasource where satellite_id = 1 and name = 'Amazon Web Services') where satellite_id = 1 and name = 'Scientific Data Hub';"
}

function updateWebConfigParams()
{
    # Set the port 8082 for the dashboard services URL
    sed -i -e "s|static \$DEFAULT_SERVICES_URL = \x27http:\/\/localhost:8080\/dashboard|static \$DEFAULT_SERVICES_URL = \x27http:\/\/localhost:8082\/dashboard|g" /var/www/html/ConfigParams.php
    sed -i -e "s|static \$DEFAULT_SERVICES_URL = \x27http:\/\/localhost:8081\/dashboard|static \$DEFAULT_SERVICES_URL = \x27http:\/\/localhost:8082\/dashboard|g" /var/www/html/ConfigParams.php

    REST_SERVER_PORT=$(sed -n 's/^server.port =//p' ${TARGET_SERVICES_DIR}/config/services.properties | sed -e 's/\r//g')
    # Strip leading space.
    REST_SERVER_PORT="${REST_SERVER_PORT## }"
    # Strip trailing space.
    REST_SERVER_PORT="${REST_SERVER_PORT%% }"
     if [[ !  -z  $REST_SERVER_PORT  ]] ; then
        sed -i -e "s|static \$DEFAULT_REST_SERVICES_URL = \x27http:\/\/localhost:8080|static \$DEFAULT_REST_SERVICES_URL = \x27http:\/\/localhost:$REST_SERVER_PORT|g" /var/www/html/ConfigParams.php
     fi

    if [[ ! -z $DB_NAME ]] ; then
        sed -i -e "s|static \$DEFAULT_DB_NAME = \x27sen2agri|static \$DEFAULT_DB_NAME = \x27${DB_NAME}|g" /var/www/html/ConfigParams.php
    fi

}

function resetDownloadFailedProducts()
{
    echo "Resetting failed downloaded products from downloader_history ..."
    sudo -u postgres psql $DB_NAME -c "update downloader_history set no_of_retries = '0' where status_id = '3' "
    sudo -u postgres psql $DB_NAME -c "update downloader_history set no_of_retries = '0' where status_id = '4' "
    sudo -u postgres psql $DB_NAME -c "update downloader_history set status_id = '3' where status_id = '4' "
    echo "Resetting failed downloaded products from downloader_history ... Done!"
}

function run_migration_scripts()
{
   local curPath=$1
   local dbName=$2
   #for each sql scripts found in this folder
   for scriptName in "$curPath"/*.sql
   do
        scriptToExecute=${scriptName}
        ## perform execution of each sql script
        echo "Executing SQL script: $scriptToExecute"
        cat "$scriptToExecute" | su - postgres -c 'psql '${dbName}''
   done
}

systemctl stop sen2agri-scheduler sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-demmaccs sen2agri-sentinel-downloader.timer sen2agri-landsat-downloader.timer sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-services

saveOldDownloadCredentials

yum -y install python-dateutil
yum -y update postgis2_94 geos
yum -y install ../rpm_binaries/*.rpm

DB_NAME=$(get_install_config_property "DB_NAME")
if [ -z "$DB_NAME" ]; then
    DB_NAME="sen2agri"
fi

echo "$DB_NAME"

TARGET_SERVICES_DIR="/usr/share/sen2agri/sen2agri-services"
#if [ "$DB_NAME" != "sen2agri" ] ; then
#    if [ -d "/usr/share/sen2agri/${DB_NAME}-services" ] ; then
#        TARGET_SERVICES_DIR="/usr/share/sen2agri/${DB_NAME}-services"
#    fi
#fi

install_sen2agri_services

ldconfig

if [ "$DB_NAME" == "sen2agri" ] ; then
    cat migrations/migration-1.3-1.3.1.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.3.1-1.4.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.4-1.5.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.5-1.6.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.6-1.6.2.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.6.2-1.7.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.7-1.8.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.8.0-1.8.1.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.8.1-1.8.2.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.8.2-1.8.3.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-1.8.3-2.0.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-2.0.0-2.0.1.sql | su -l postgres -c "psql $DB_NAME"
    cat migrations/migration-2.0.1-2.0.2.sql | su -l postgres -c "psql $DB_NAME"
else
    run_migration_scripts "migrations/${DB_NAME}" "${DB_NAME}"
fi

systemctl daemon-reload

mkdir -p /mnt/archive/reference_data
echo "Copying reference data"
if [ -d ../reference_data/ ]; then
    cp -rf ../reference_data/* /mnt/archive/reference_data
fi

# Update the port in /var/www/html/ConfigParams.php as version 1.8 had 8080 instead of 8081
updateWebConfigParams

if [ "$DB_NAME" == "sen2agri" ] ; then
    updateDownloadCredentials

    # Enable SciHub as the download datasource
    enableSciHubDwnDS

    # Reset the download failed products
    resetDownloadFailedProducts
else
    # Install and config SNAP
#    wget http://step.esa.int/downloads/6.0/installers/esa-snap_sentinel_unix_6_0.sh && \
#    cp -f esa-snap_sentinel_unix_6_0.sh /tmp/ && \
#    chmod +x /tmp/esa-snap_sentinel_unix_6_0.sh && \
#    /tmp/esa-snap_sentinel_unix_6_0.sh -q && \
#    /opt/snap/bin/snap --nosplash --nogui --modules --update-all
#    rm -f ./esa-snap_sentinel_unix_6_0.sh /tmp/esa-snap_sentinel_unix_6_0.sh
#    if [ ! -h /usr/local/bin/gpt ]; then sudo ln -s /opt/snap/bin/gpt /usr/local/bin/gpt;fi
#
#    cp -f ${GPT_CONFIG_FILE} /opt/snap/bin/
#
#    # Install R-devel
#    yum install -y R-devel
#    echo 'install.packages(c("e1071", "caret", "dplyr", "gsubfn", "ranger", "readr", "smotefamily"), repos = c(CRAN = "https://cran.rstudio.com"))' | Rscript -

    # Install Miniconda and the environment for the execution of processors
    SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
    cp "${SCRIPTPATH}/../tools/miniconda/Miniconda3-latest-Linux-x86_64.sh" "/mnt/archive/"
    cp "${SCRIPTPATH}/../tools/miniconda/sen4cap_conda.yml" "/mnt/archive/"

    echo "Installing conda ..."
    if [ -f /home/sen2agri-service/miniconda3/etc/profile.d/conda.sh ] ; then
        echo "/home/sen2agri-service/miniconda3/etc/profile.d/conda.sh found ..."
        echo "Miniconda already installed for user sen2agri-service. Nothing to do ..."
    else
        sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b"
        # sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b -p /mnt/archive/sen4cap_miniconda/miniconda3/"
        # sudo -u sen2agri-service bash -c 'echo ". /mnt/archive/sen4cap_miniconda/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
        echo "Updating bashrc ..."
        sudo -u sen2agri-service bash -c 'echo ". /home/sen2agri-service/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
    fi
    echo "Updating R packages..."
    Rscript - <<- EOF
    packages <- c("e1071", "caret", "dplyr", "gsubfn", "ranger", "readr", "smotefamily", "caTools", "tidyverse", "data.table")
    diff <- setdiff(packages, rownames(installed.packages()))
    if (length(diff) > 0) {
        install.packages(diff, repos = c(CRAN = "https://cran.rstudio.com"))
    }
EOF

    echo "Setting report_errors to false..."
    sudo su -l sen2agri-service -c bash -c "conda config --set report_errors false"
    CUR_SEN4CAP_ENV_VAL=$(sudo su -l sen2agri-service -c bash -c "conda info --envs" | grep sen4cap)
    if [ -z "$CUR_SEN4CAP_ENV_VAL" ]; then
        echo "Creating sen4cap conda environment ..."
        sudo su -l sen2agri-service -c bash -c "conda env create --file=/mnt/archive/sen4cap_conda.yml"
        echo "Printing current environments ..."
        sudo su -l sen2agri-service -c bash -c "conda info --envs"
    else
        echo "sen4cap conda environment already exists. Nothing to do ..."
        echo "Environments:"
        sudo su -l sen2agri-service -c bash -c "conda info --envs"
    fi

    rm "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh"
    rm "/mnt/archive/sen4cap_conda.yml"

fi

#systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-demmaccs sen2agri-sentinel-downloader.timer sen2agri-landsat-downloader.timer sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler

# Do not start anymore the old downloaders but instead is started the sen2agri-services
systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-demmaccs sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler sen2agri-services


