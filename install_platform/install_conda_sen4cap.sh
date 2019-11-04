#!/bin/bash
#set -x #echo on

: ${INSTAL_CONFIG_FILE:="./config/install_config.conf"}

#----------------SEN2AGRI POSTGRESQL DATABASE NAME-----------------------------------------#
: ${SEN2AGRI_DATABASE_NAME:="sen2agri"}

function get_install_config_property
{
    grep "^$1=" "${INSTAL_CONFIG_FILE}" | cut -d'=' -f2 | sed -e 's/\r//g'
}

#-----------------------------------------------------------#
function install_additional_packages()
{
    DB_NAME=$(get_install_config_property "DB_NAME")
    if [ -z "$DB_NAME" ]; then
        DB_NAME="sen2agri"
    fi
    SEN2AGRI_DATABASE_NAME=${DB_NAME}

    if ! [[ "${SEN2AGRI_DATABASE_NAME}" == "sen2agri" ]] ; then
        # Install Miniconda and the environment for the execution of processors
        SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
        cp "${SCRIPTPATH}/../tools/miniconda/Miniconda3-latest-Linux-x86_64.sh" "/mnt/archive/"
        cp "${SCRIPTPATH}/../tools/miniconda/sen4cap_conda.yml" "/mnt/archive/"
        
        sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b"
        # sudo su -l sen2agri-service -c bash -c "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh -b -p /mnt/archive/sen4cap_miniconda/miniconda3/"
        # sudo -u sen2agri-service bash -c 'echo ". /mnt/archive/sen4cap_miniconda/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
        sudo -u sen2agri-service bash -c 'echo ". /home/sen2agri-service/miniconda3/etc/profile.d/conda.sh" >> /home/sen2agri-service/.bashrc'
        sudo su -l sen2agri-service -c bash -c "conda config --set report_errors false"
        sudo su -l sen2agri-service -c bash -c "conda env create --file=/mnt/archive/sen4cap_conda.yml"
        sudo su -l sen2agri-service -c bash -c "conda info --envs"
        
        rm "/mnt/archive/Miniconda3-latest-Linux-x86_64.sh"
        rm "/mnt/archive/sen4cap_conda.yml"
    fi
}

#-----------------------------------------------------------#
####  ADDITIONAL PACKAGES      INSTALL                  #####
#-----------------------------------------------------------#
install_additional_packages

