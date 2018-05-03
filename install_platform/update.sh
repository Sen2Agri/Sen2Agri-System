#!/bin/sh

HAS_S2AGRI_SERVICES=false
function install_sen2agri_services() 
{
    if [ ! -d "/usr/share/sen2agri/sen2agri-services" ]; then
        if [ -f ../sen2agri-services/sen2agri-services*.zip ]; then
            zipArchive=$(ls -at ../sen2agri-services/sen2agri-services*.zip| head -n 1)
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
        else
            echo "No sen2agri-services zip archive provided in ../sen2agri-services"
            echo "Exiting now"
            exit 1
        fi
    else
        echo "sen2agri-services already exist in /usr/share/sen2agri/sen2agri-services"
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
        sed -i '/SciHubDataSource.username=/c\SciHubDataSource.username='"$SCIHUB_USER" /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties
        sed -i '/SciHubDataSource.password=/c\SciHubDataSource.password='"$SCIHUB_PASS" /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties
        sed -i '/USGSDataSource.username=/c\USGSDataSource.username='"$USGS_USER" /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties
        sed -i '/USGSDataSource.password=/c\USGSDataSource.password='"$USGS_PASS" /usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties
    fi
}

systemctl stop sen2agri-scheduler sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-demmaccs sen2agri-sentinel-downloader.timer sen2agri-landsat-downloader.timer sen2agri-demmaccs.timer sen2agri-monitor-agent

saveOldDownloadCredentials

yum -y install python-dateutil
yum -y update postgis2_94 geos
yum -y install ../rpm_binaries/*.rpm

install_sen2agri_services

ldconfig

cat migrations/migration-1.3-1.3.1.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.3.1-1.4.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.4-1.5.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.5-1.6.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.6-1.6.2.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.6.2-1.7.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.7-1.8.sql | su -l postgres -c "psql sen2agri"
cat migrations/migration-1.8-1.8.1.sql | su -l postgres -c "psql sen2agri"

systemctl daemon-reload

mkdir -p /mnt/archive/reference_data
echo "Copying reference data"
if [ -d ../reference_data/ ]; then
    cp -rf ../reference_data/* /mnt/archive/reference_data
fi

updateDownloadCredentials

#systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-demmaccs sen2agri-sentinel-downloader.timer sen2agri-landsat-downloader.timer sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler

# Do not start anymore the old downloaders but instead is started the sen2agri-services
systemctl start sen2agri-executor sen2agri-orchestrator sen2agri-http-listener sen2agri-demmaccs sen2agri-demmaccs.timer sen2agri-monitor-agent sen2agri-scheduler sen2agri-services


