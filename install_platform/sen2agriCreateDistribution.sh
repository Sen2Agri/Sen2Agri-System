#!/bin/bash
#set -x #echo on

##
## SCRIPT: CREATE PLATFORM DISTRIBUTION SEN2AGRI
## SCRIPT STEPS
##     - CREATES DISTRIBUTION FOLDER
##     - COPIES INSTALL SCRIPTS,RPMs, CONFIG FILES INTO SPECIFIC LOCATIONS
################################################################################################
## SCRIPT USAGE:
##
## open a terminal and type:
## $ /path/to/scripts/sen2agri/install_platform/sen2agriCreateDistribution.sh
################################################################################################
### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_INSTALL_DIR:="Sen2AgriDistribution"}
: ${DEFAULT_PATH:=$(pwd)}
: ${SOURCES_DIR_PATH:=""}
: ${RPM_DIR_PATH:=""}
#------------------------------------------------------------------------------------------#
function create_distrib_dir()
{
   ##go to default dir
   cd ${DEFAULT_PATH}

   ##create working dir
   if [ ! -d ${DEFAULT_INSTALL_DIR} ]; then
      mkdir -p ${DEFAULT_INSTALL_DIR}
   fi
}
#------------------------------------------------------------------------------------------#
function get_SEN2AGRI_sources_path()
{
   ## build script will reside to sen2agri/packaging
   #get script path
   script_path=$(dirname $0)

   ##go in the folder sen2agri/packaging and exit up one folder into the source root dir sen2agri
   cd $script_path
   cd ..

   #save the sources path
   SOURCES_DIR_PATH=$(pwd)
}
#------------------------------------------------------------------------------------------#
function prepare_distribution()
{
   ##create working dir
   create_distrib_dir

   ##get sources path
   get_SEN2AGRI_sources_path

   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/maccs/core
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/maccs/cots

   #----------------------rpm_binaries dir--------------------------------------------------------------------#

   ##create dir "rpm_binaries" inside distribution folder Sen2AgriDistribution
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/rpm_binaries

   ##create subdir "slurm" inside distribution folder Sen2AgriDistribution/rpm_binaries
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/rpm_binaries/slurm

   ##copy built RPMs for Sen2agri platform into  folder /rpm_binaries
   cp -f ${RPM_DIR_PATH}/*.rpm  ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/rpm_binaries

   ##copy RPMs for SLURM  into  folder /rpm_binaries/slurm
   cp -f ${SOURCES_DIR_PATH}/install_platform/slurm_rpm_package/*.rpm  ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/rpm_binaries/slurm

   #----------------------install_scripts dir--------------------------------------------------------------------#
   ##create dir "install_script" inside distribution folder Sen2AgriDistribution
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script

   ##create dir "config" inside distribution folder Sen2AgriDistribution/install_script
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/config

   #copy content of dir "config_files" found at sent2agri/install_platform into created install_script/config
   cp -f ${SOURCES_DIR_PATH}/install_platform/config_files/*.conf ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/config

   #copy dir "database" found at sent2agri into subfolder created install_script/
   cp -rf ${SOURCES_DIR_PATH}/database ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/

   #copy installation scripts *.sh found at sent2agri/install_platform/ into subfolder created install_script/
   cp -f ${SOURCES_DIR_PATH}/install_platform/*.sh ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/

   ##create dir "adapters" inside distribution folder Sen2AgriDistribution/install_script
   mkdir -p ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/adapters

   #copy adapters *.nbm found at sent2agri/snap-adapters/ into subfolder created install_script/adapters
   cp -f ${SOURCES_DIR_PATH}/snap-adapters/*.nbm ${DEFAULT_PATH}/${DEFAULT_INSTALL_DIR}/install_script/adapters

}

###########################################################
##### MAIN                                              ###
###########################################################
##check nb of arguments of the script,
# script argument should indicate the RPM-files folder
if [[ $# -eq 0 ]] ; then
    echo 'Specify RPMs folder full path'
    echo 'Usage: : $0 ARGUMENT"'
    exit 1
fi

#save argument and transform to absolute path
RPM_DIR_PATH=$(cd "$1";pwd)

#prepare distribution
prepare_distribution
