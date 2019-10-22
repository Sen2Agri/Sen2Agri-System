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
    if [ ! -d "/usr/share/sen2agri/sen2agri-services" ] || [ ! "$(ls -A /usr/share/sen2agri/sen2agri-services)" ] ; then
        if [ -f ../sen2agri-services/${SERVICES_ARCHIVE} ]; then
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
        else
            echo "No sen2agri-services zip archive provided in ../sen2agri-services"
            echo "Exiting now"
            exit 1
        fi
    else
        echo "sen2agri-services already exist in /usr/share/sen2agri/sen2agri-services"
        if [ -d "/usr/share/sen2agri/sen2agri-services/bin" ] && [ -d "/usr/share/sen2agri/sen2agri-services/config" ] ; then 
            if [ -f ../sen2agri-services/${SERVICES_ARCHIVE} ]; then
                echo "Updating /usr/share/sen2agri/sen2agri-services/lib folder ..."
                mkdir -p /usr/share/sen2agri/sen2agri-services/lib && rm -f /usr/share/sen2agri/sen2agri-services/lib/*.jar && unzip -o ${zipArchive} 'lib/*' -d /usr/share/sen2agri/sen2agri-services
                echo "Updating /usr/share/sen2agri/sen2agri-services/modules folder ..."
                mkdir -p /usr/share/sen2agri/sen2agri-services/modules && rm -f /usr/share/sen2agri/sen2agri-services/modules/*.jar && unzip -o ${zipArchive} 'modules/*' -d /usr/share/sen2agri/sen2agri-services
                mkdir -p /usr/share/sen2agri/sen2agri-services/scripts
                
                if [ -f /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties ] ; then 
                    mv /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties /usr/share/sen2agri/sen2agri-services/config/services.properties
                fi
                if [ -f /usr/share/sen2agri/sen2agri-services/config/application.properties ] ; then 
                    cp -f /usr/share/sen2agri/sen2agri-services/config/application.properties /usr/share/sen2agri/sen2agri-services/config/application.properties.bkp
                fi
                # update the application.properties file even if some user changes might be lost
                unzip -o ${zipArchive} 'config/application.properties' -d /usr/share/sen2agri/sen2agri-services/
            else
                echo "No archive sen2agri-services-YYY.zip was found in the installation package. sen2agri-services will not be updated!!!"
            fi
        else 
            echo "ERROR: no bin or config folder were found in the folder /usr/share/sen2agri/sen2agri-services/. No update will be made!!!"
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
        sed -i '/SciHubDataSource.username=/c\SciHubDataSource.username='"$SCIHUB_USER" /usr/share/sen2agri/sen2agri-services/config/services.properties
        sed -i '/SciHubDataSource.password=/c\SciHubDataSource.password='"$SCIHUB_PASS" /usr/share/sen2agri/sen2agri-services/config/services.properties
        sed -i '/USGSDataSource.username=/c\USGSDataSource.username='"$USGS_USER" /usr/share/sen2agri/sen2agri-services/config/services.properties
        sed -i '/USGSDataSource.password=/c\USGSDataSource.password='"$USGS_PASS" /usr/share/sen2agri/sen2agri-services/config/services.properties
    fi
}

function enableSciHubDwnDS()
{
    echo "Disabling Amazon datasource ..."
    sed -i '/SciHubDataSource.Sentinel2.scope=1/c\SciHubDataSource.Sentinel2.scope=3' /usr/share/sen2agri/sen2agri-services/config/services.properties
    sed -i '/AWSDataSource.Sentinel2.enabled=true/c\AWSDataSource.Sentinel2.enabled=false' /usr/share/sen2agri/sen2agri-services/config/services.properties
    
    sed -i 's/AWSDataSource.Sentinel2.local_archive_path=/SciHubDataSource.Sentinel2.local_archive_path=/g' /usr/share/sen2agri/sen2agri-services/config/services.properties
    sed -i 's/AWSDataSource.Sentinel2.fetch_mode=/SciHubDataSource.Sentinel2.fetch_mode=/g' /usr/share/sen2agri/sen2agri-services/config/services.properties
    
    sudo -u postgres psql $DB_NAME -c "update datasource set scope = 3 where satellite_id = 1 and name = 'Scientific Data Hub';"
    sudo -u postgres psql $DB_NAME -c "update datasource set enabled = 'false' where satellite_id = 1 and name = 'Amazon Web Services';"
    echo "Disabling Amazon datasource ... Done!"
    
#    sudo -u postgres psql sen2agri -c "update datasource set local_root = (select local_root from datasource where satellite_id = 1 and name = 'Amazon Web Services') where satellite_id = 1 and name = 'Scientific Data Hub';"
#    sudo -u postgres psql sen2agri -c "update datasource set fetch_mode = (select fetch_mode from datasource where satellite_id = 1 and name = 'Amazon Web Services') where satellite_id = 1 and name = 'Scientific Data Hub';"
}

function updateWebRestPort()
{
    # Set the port 8082 for the dashboard services URL 
    sed -i -e "s|static \$SERVICES_URL = \x27http:\/\/localhost:8080\/dashboard|static \$SERVICES_URL = \x27http:\/\/localhost:8082\/dashboard|g" /var/www/html/ConfigParams.php
    sed -i -e "s|static \$SERVICES_URL = \x27http:\/\/localhost:8081\/dashboard|static \$SERVICES_URL = \x27http:\/\/localhost:8082\/dashboard|g" /var/www/html/ConfigParams.php
    
    REST_SERVER_PORT=$(sed -n 's/^server.port =//p' /usr/share/sen2agri/sen2agri-services/config/services.properties)
    # Strip leading space.
    REST_SERVER_PORT="${REST_SERVER_PORT## }"
    # Strip trailing space.
    REST_SERVER_PORT="${REST_SERVER_PORT%% }"
     if [[ !  -z  $REST_SERVER_PORT  ]] ; then
        sed -i -e "s|static \$REST_SERVICES_URL = \x27http:\/\/localhost:8080|static \$REST_SERVICES_URL = \x27http:\/\/localhost:$REST_SERVER_PORT|g" /var/www/html/ConfigParams.php
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

install_sen2agri_services

ldconfig

DB_NAME=$(get_install_config_property "DB_NAME")
if [ -z "$DB_NAME" ]; then
    DB_NAME="sen2agri"
fi

echo "$DB_NAME"

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
else 
    run_migration_scripts "migrations/${DB_NAME}" "${DB_NAME}"
fi

systemctl daemon-reload

mkdir -p /mnt/archive/reference_data
echo "Copying reference data"
if [ -d ../reference_data/ ]; then
    cp -rf ../reference_data/* /mnt/archive/reference_data
fi

if [ "$DB_NAME" == "sen2agri" ] ; then 
    updateDownloadCredentials

    # Update the port in /var/www/html/ConfigParams.php as version 1.8 had 8080 instead of 8081
    updateWebRestPort

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
    
    sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b"
    # sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b -p /mnt/archive/sen4cap_miniconda/miniconda3/"
    sudo -u sen2agri-service bash -c 'echo ". /home/sen2agri-service/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
    sudo su -l sen2agri-service -c bash -c "conda config --set report_errors false"
    sudo su -l sen2agri-service -c bash -c "conda env create --file=/mnt/archive/sen4cap_conda.yml"
    sudo su -l sen2agri-service -c bash -c "conda info --envs"
    
    rm "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh"
    rm "/mnt/archive/sen4cap_conda.yml"
    
fi

#systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-demmaccs sen2agri-sentinel-downloader.timer sen2agri-landsat-downloader.timer sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler

# Do not start anymore the old downloaders but instead is started the sen2agri-services
systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-demmaccs sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler sen2agri-services


