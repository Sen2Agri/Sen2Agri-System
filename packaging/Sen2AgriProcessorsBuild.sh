#!/bin/bash
#set -x #echo on

##
## SCRIPT: BUILD SEN2AGRI PROCESSORS
##
##
## SCRIPT STEPS
##     - CREATE DIR TREE: Sen2AgriProcessors/install, Sen2AgriProcessors/build and Sen2AgriProcessors/rpm_binaries
##     - COMPILE AND INSTALL SEN2AGRI PROCESSORS
##     - RPM GENERATION FOR COMPILED SEN2AGRI PROCESSORS
##     - RPM GENERATION FOR DEMMACCS AND DOWNLOADERS
###########################CONFIG PART###########################################################
### DEPENDENCIES FOR GENERATED RPM PACKAGES
: ${PLATFORM_INSTALL_OTHER_DEP:="-d otb -d gdal-python"}

### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_DIR:=$(pwd)}
: ${PLATFORM_NAME_DIR:="Sen2AgriProcessors"}
: ${INSTALL_DIR:="install"}
: ${RPM_DIR:="rpm_binaries"}
: ${BUILD_DIR:="build"}
: ${PROC_VERSION:="2.1.36"}
: ${DOWNL_DEM_VERSION:="2.1.36"}
: ${WORKING_DIR_INSTALL:=${PLATFORM_NAME_DIR}/${INSTALL_DIR}}
: ${WORKING_DIR_RPM:=${PLATFORM_NAME_DIR}/${RPM_DIR}}
: ${WORKING_DIR_BUILD:=${PLATFORM_NAME_DIR}/${BUILD_DIR}}
: ${SOURCES_DIR_PATH:=""}
: ${PROC_INSTALL_PATH:="${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/processors-install"}
: ${DOWNL_DEM_INSTALL_PATH:="${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/downloaders-dem-install"}
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
function compile_SEN2AGRI_processors()
{
   mkdir ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/sen2agri-processors-build
   cd ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/sen2agri-processors-build

   ##configure
   cmake ${SOURCES_DIR_PATH}/sen2agri-processors -DCMAKE_INSTALL_PREFIX=${PROC_INSTALL_PATH} -DCMAKE_BUILD_TYPE=RelWithDebInfo

   ##compile
   make -j$(grep -c "^processor" /proc/cpuinfo)

   ##install
   make install
}
#-----------------------------------------------------------#
function build_SEN2AGRI_processors_RPM_Package()
{
   ## add script for mosaication into the processors package
   cp -f ${SOURCES_DIR_PATH}/sen2agri-processors/aggregate_tiles/*.py ${PROC_INSTALL_PATH}/usr/bin

   ## add script for validity checking of the created products
   cp -f ${SOURCES_DIR_PATH}/sen2agri-processors/validity_checker/*.py ${PROC_INSTALL_PATH}/usr/bin

   ##create a temporary dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_processors

   ##build RPM package
   fpm -s dir -t rpm -n sen2agri-processors -v ${PROC_VERSION} -C ${PROC_INSTALL_PATH}/ ${PLATFORM_INSTALL_OTHER_DEP} \
       --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_processors -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/sen2agri-processors-VERSION.centos7.ARCH.rpm \
       usr etc

   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_processors
}
#-----------------------------------------------------------#
function build_SEN2AGRI_downloaders_demmacs_RPM_Package()
{
   ###########################################
   #DOWNLOADERS
   ###########################################

   ##downloaders/demmaccs scripts will be installed in folder : usr/share/sen2agri/sen2agri-downloaders or usr/share/sen2agri/sen2agri-demmaccs
   #mkdir -p ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-downloaders
   mkdir -p ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-demmaccs

   ##downloaders/demmaccs services will be installed in folder : usr/lib/systemd/system
   mkdir -p ${DOWNL_DEM_INSTALL_PATH}/usr/lib/systemd/system

   ##downloaders/demmaccs common *.py scripts will reside to  /usr/lib/python2.7/site-packages/

   ##create folder tree into install folder : usr/
   mkdir -p ${DOWNL_DEM_INSTALL_PATH}/usr/lib/python2.7/site-packages

   ###########################################
   #DOWNLOADERS
   ###########################################
   #NOTE: since 1.8 these downloaders are no longer used. The sen2agri-services are used instead
   ###put downloaders lib dir into the install folder :usr/share/sen2agri/sen2agri-downloaders
#   cp -rf ${SOURCES_DIR_PATH}/sen2agri-downloaders/lib ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-downloaders

   ###put downloaders *.py files into the install folder :usr/share/sen2agri/sen2agri-downloaders
#   cp -f ${SOURCES_DIR_PATH}/sen2agri-downloaders/*.py ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-downloaders

   ###put downloaders *.jar files into the install folder :usr/share/sen2agri/sen2agri-downloaders
#   cp -f ${SOURCES_DIR_PATH}/sen2agri-downloaders/*.jar ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-downloaders

   ###put downloaders *.txt files into the install folder :usr/share/sen2agri/sen2agri-downloaders
#   cp -f ${SOURCES_DIR_PATH}/sen2agri-downloaders/*.txt ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-downloaders

   ###put downloaders services files into the install folder :/usr/lib/systemd/system
#   cp -f ${SOURCES_DIR_PATH}/sen2agri-downloaders/dist/* ${DOWNL_DEM_INSTALL_PATH}/usr/lib/systemd/system

   ###########################################
   #DEMMACCS
   ###########################################
   ###put demmaccs script files into the install folder  usr/share/sen2agri/sen2agri-demmaccs
   cp -f ${SOURCES_DIR_PATH}/sen2agri-processors/DEM-WB/test/*.py  ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-demmaccs

   cp -rf ${SOURCES_DIR_PATH}/sen2agri-processors/DEM-WB/UserConfiguration ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri/sen2agri-demmaccs

   cp -rf ${SOURCES_DIR_PATH}/sen2agri-processors/DEM-WB/wrs2_descending ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri
   cp -rf ${SOURCES_DIR_PATH}/sen2agri-processors/DEM-WB/l8_alignment ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri

   cp -f ${SOURCES_DIR_PATH}/sen2agri-processors/fix_utm_proj/fix_utm_proj.py ${DOWNL_DEM_INSTALL_PATH}/usr/share/sen2agri

   ###put demmaccs services files into the install folder :/usr/lib/systemd/system
   cp -f ${SOURCES_DIR_PATH}/sen2agri-processors/DEM-WB/test/dist/* ${DOWNL_DEM_INSTALL_PATH}/usr/lib/systemd/system

   ###########################################
   #DOWNLOADERS DEMMACCS COMMON
   ###########################################
   cp -f ${SOURCES_DIR_PATH}/python-libs/*.py  ${DOWNL_DEM_INSTALL_PATH}/usr/lib/python2.7/site-packages

   ###########################################
   #PACKAGE BUILD
   ###########################################

   ##create a temporary dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_download_demmacs

   ##build RPM package
   fpm -s dir -t rpm -n sen2agri-downloaders-demmaccs -v ${DOWNL_DEM_VERSION} -C ${DOWNL_DEM_INSTALL_PATH}/ \
       -d python-beautifulsoup4 \
       --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_download_demmacs \
       -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/sen2agri-downloaders-demmaccs-VERSION.centos7.ARCH.rpm \
       usr

#       --config-files usr/share/sen2agri/sen2agri-downloaders/usgs.txt \
#       --config-files usr/share/sen2agri/sen2agri-downloaders/apihub.txt \

   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_download_demmacs
}
#-----------------------------------------------------------#
function build_dir_tree()
{
   ##set PATH env variable to /usr/bin to avoid
   # cmake finding /lib/cmake before /usr/lib/cmake
   export PATH="/usr/bin:$PATH"

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

   ##create build dir
   if [ ! -d ${BUILD_DIR} ]; then
      mkdir -p ${BUILD_DIR}
   fi

   ##exit from platform dir
   cd ..
}

###########################################################
##### PREPARE ENVIRONEMENT FOR BUILDING PROCESSORS     ###
###########################################################
##create folder tree: build, install and rpm
build_dir_tree

##get sources path
get_SEN2AGRI_sources
###########################################################
#####  PROCESSORS build, install and RPM generation  ######
###########################################################
## SEN2AGRI processors sources compile and install
compile_SEN2AGRI_processors

##create RPM package
build_SEN2AGRI_processors_RPM_Package
###########################################################
#####  Downloaders-Demmaccs RPM generation           ######
###########################################################
build_SEN2AGRI_downloaders_demmacs_RPM_Package
