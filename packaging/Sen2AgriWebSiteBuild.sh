#!/bin/bash
#set -x #echo on

##
## SCRIPT: BUILD SEN2AGRI WEBSITE
##
##
## SCRIPT STEPS
##     - CREATE DIR TREE on the current path: Sen2AgriWebSite/install and Sen2AgriWebSite/rpm_binaries
##     - COPY SEN2AGRI WEBSITE FILES into install folder
##     - RPM GENERATION FOR SEN2AGRI WEBSITE FILES
###########################CONFIG PART###########################################################
### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_DIR:=$(pwd)}
: ${PLATFORM_NAME_DIR:="Sen2AgriWebSite"}
: ${RPM_DIR:="rpm_binaries"}
: ${INSTALL_DIR:="install"}
: ${SITE_VERSION:="1.7.1"}
: ${WEB_DIR:="sen2agri-dashboard"}
: ${SOURCES_DIR_PATH:=""}
: ${WORKING_DIR_RPM:=${PLATFORM_NAME_DIR}/${RPM_DIR}}
: ${WORKING_DIR_INSTALL:=${PLATFORM_NAME_DIR}/${INSTALL_DIR}}
################################################################################################
#-----------------------------------------------------------#
function get_SEN2AGRI_sources()
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
#-----------------------------------------------------------#
function build_SEN2AGRI_website_RPM_Package()
{
   ##go to the website dir
   WEBSITE_FILE_PATH=${SOURCES_DIR_PATH}/${WEB_DIR}

   ##create a folder var
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/var

   ##create folder www
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/var/www

   cp -rf ${WEBSITE_FILE_PATH}/ ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/var/www/

   mv ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/var/www/${WEB_DIR} ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/var/www/html

   ##create a temporary dir for RPM generation
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_site

   ##build RPM package
   fpm -s dir -t rpm -n sen2agri-website -v ${SITE_VERSION} -C ${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/ \
   --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_site -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/sen2agri-website-VERSION.centos7.ARCH.rpm var

   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_site
}

#-----------------------------------------------------------#
function build_dir_tree()
{
   ##go to default dir
   cd ${DEFAULT_DIR}

   ##create platform dir
   if [ ! -d ${PLATFORM_NAME_DIR} ]; then
      mkdir -p ${PLATFORM_NAME_DIR}
   fi

   ##go into platform dir
   cd ${PLATFORM_NAME_DIR}

   ##create install dir
   if [ ! -d ${INSTALL_DIR} ]; then
      mkdir -p ${INSTALL_DIR}
   fi

   ##create rpm dir
   if [ ! -d ${RPM_DIR} ]; then
      mkdir -p ${RPM_DIR}
   fi

   ##exit from platform dir
   cd ..
}

###########################################################
##### PREPARE ENVIRONEMENT FOR SEN2AGRI WEBSITE         ###
###########################################################
##create folder tree: build, install and rpm
build_dir_tree

##get sources path
get_SEN2AGRI_sources
###########################################################
#####  SEN2AGRI WEBSITE RPM generation               ######
###########################################################
##build RPM for SEN2AGRI website
build_SEN2AGRI_website_RPM_Package

