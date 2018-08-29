#!/bin/sh

function install_gipp_files() 
{
    GIPP_PATH=$(sudo -u postgres psql sen2agri -qtA -c "select value from config where key='demmaccs.gips-path';")
    GIPP_PATH=${GIPP_PATH%/}
    if ! [[ -z "${GIPP_PATH}" ]] ; then
        echo "GIPP files found configured in ${GIPP_PATH}. Replacing ..."
        DATE=$(date +"%Y_%m_%dT%H_%M_%S")
        echo "Backing up ${GIPP_PATH} to ${GIPP_PATH}_OLD_${DATE}"
        mv "${GIPP_PATH}" "${GIPP_PATH}_OLD_${DATE}"
        mkdir -p "${GIPP_PATH}"
        echo "Copying new GIPP files to ${GIPP_PATH} ..."
        cp -fR ../gipp/* "${GIPP_PATH}"
        echo "Done!"
    fi
}

systemctl stop sen2agri-demmaccs.timer sen2agri-demmaccs

install_gipp_files

systemctl start sen2agri-demmaccs.timer


