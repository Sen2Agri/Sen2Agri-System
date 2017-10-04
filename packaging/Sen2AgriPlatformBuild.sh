#!/bin/bash
#set -x #echo on

##
## SCRIPT: BUILD SEN2AGRI OTB
##
##
## SCRIPT STEPS
##     - CREATE DIR TREE: Sen2AgriPlatform/install, Sen2AgriPlatform/build and Sen2AgriPlatform/rpm_binaries
##     - RETRIEVE COMPILE AND INSTALL OTB APP
##     - RPM GENERATION FOR COMPILED OTB APP
################################################################################################
###########################CONFIG PART###########################################################
### URLs FOR RETRIEVING SOURCES PACKAGES
: ${OTB_URL:="https://github.com/Sen2Agri/OTB.git"}
: ${OTB_MOSAIC_URL:="https://github.com/remicres/otb-mosaic.git"}

### DEPENDENCIES FOR GENERATED RPM PACKAGES
PLATFORM_INSTALL_DEP=(-d boost -d curl -d expat -d fftw -d gdal -d geos -d libgeotiff -d libjpeg-turbo -d libsvm -d muParser -d opencv -d openjpeg2 -d openjpeg2-tools -d pcre -d libpng -d proj -d proj-epsg -d python -d swig -d libtiff -d tinyxml -d qt5-qtbase -d qt5-qtbase-postgresql -d qt-x11 -d gsl)

### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_DIR:=$(pwd)}
: ${PLATFORM_NAME_DIR:="Sen2AgriPlatform"}
: ${INSTALL_DIR:="install"}
: ${RPM_DIR:="rpm_binaries"}
: ${BUILD_DIR:="build"}
: ${WORKING_DIR_INSTALL:=${PLATFORM_NAME_DIR}/${INSTALL_DIR}}
: ${WORKING_DIR_RPM:=${PLATFORM_NAME_DIR}/${RPM_DIR}}
: ${WORKING_DIR_BUILD:=${PLATFORM_NAME_DIR}/${BUILD_DIR}}
: ${OTB_VERSION:="5.0"}
OTB_ITERATION=3
: ${OTB_INSTALL_PATH:="${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/otb-install"}
NUM_CPUS=$(grep -c "^processor" /proc/cpuinfo)
################################################################################################
#-----------------------------------------------------------#
function compile_OTB_package()
{
   ##download OTB
   curl -LO https://github.com/Sen2Agri/OTB/archive/fixes-5.0.zip
   rm -rf OTB-fixes-5.0 ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}
   unzip fixes-5.0.zip
   rm fixes-5.0.zip
   mv OTB-fixes-5.0 ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}

   ##download MOSAIC
   local OTB_MOSAIC_SHA=c741a7bf7dbe790727b6698635f50ebe4108a454
   curl -LO https://github.com/remicres/otb-mosaic/archive/${OTB_MOSAIC_SHA}.zip
   rm -rf otb-mosaic-${OTB_MOSAIC_SHA} ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}/Modules/Remote/OTB-MOSAIC
   unzip ${OTB_MOSAIC_SHA}.zip
   rm ${OTB_MOSAIC_SHA}.zip
   mv otb-mosaic-${OTB_MOSAIC_SHA} ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}/Modules/Remote/OTB-MOSAIC

   ## go into OTB build dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}-BUILD
   cd ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}-BUILD

   ##configure OTB
   cmake ../OTB-${OTB_VERSION}/SuperBuild \
	 -DCMAKE_INSTALL_PREFIX=${OTB_INSTALL_PATH}/usr \
	 -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	 -DBUILD_TESTING=OFF \
	 -DGDAL_CONFIG=/usr/bin/gdal-config \
	 -DGDAL_INCLUDE_DIR=/usr/include/gdal \
	 -DGDAL_LIBRARY=/usr/lib64/libgdal.so \
	 -DOTB_USE_MUPARSERX=OFF \
     -DOTB_USE_LIBKML=OFF \
     -DOTB_USE_QT4=OFF \
     -DOTB_USE_SIFTFAST=OFF \
     -DOTB_WRAP_PYTHON=ON \
	 -DUSE_SYSTEM_BOOST=ON \
	 -DUSE_SYSTEM_CURL=ON \
	 -DUSE_SYSTEM_EXPAT=ON \
	 -DUSE_SYSTEM_FFTW=ON \
	 -DUSE_SYSTEM_GDAL=ON \
	 -DUSE_SYSTEM_GEOS=ON \
	 -DUSE_SYSTEM_GEOTIFF=ON \
	 -DUSE_SYSTEM_ITK=OFF \
	 -DUSE_SYSTEM_JPEG=ON \
	 -DUSE_SYSTEM_LIBSVM=ON \
	 -DUSE_SYSTEM_MUPARSER=ON \
	 -DUSE_SYSTEM_OPENCV=ON \
	 -DUSE_SYSTEM_OPENTHREADS=OFF \
	 -DUSE_SYSTEM_OSSIM=OFF \
	 -DUSE_SYSTEM_PNG=ON \
	 -DUSE_SYSTEM_SWIG=ON \
	 -DUSE_SYSTEM_TIFF=ON \
	 -DUSE_SYSTEM_TINYXML=ON \
	 -DUSE_SYSTEM_ZLIB=ON

   ## compile OTB
   make -j$NUM_CPUS
}
#-----------------------------------------------------------#
function build_OTB_RPM_Package()
{
   ## replace cmake files
   sed -i -e "s|${OTB_INSTALL_PATH}/|/|g" ${OTB_INSTALL_PATH}/usr/lib/cmake/OTB-${OTB_VERSION}/*.cmake
   sed -i -e "s|${OTB_INSTALL_PATH}/|/|g" ${OTB_INSTALL_PATH}/usr/lib/cmake/OTB-${OTB_VERSION}/Modules/*.cmake
   sed -i -e "s|${OTB_INSTALL_PATH}/|/|g" ${OTB_INSTALL_PATH}/usr/lib/pkgconfig/openthreads.pc

   ## create a temp working dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_otb

   ## build the packages specifying installing dependencies
   fpm -s dir -t rpm -n otb -v ${OTB_VERSION} --iteration $OTB_ITERATION -C ${OTB_INSTALL_PATH}/ "${PLATFORM_INSTALL_DEP[@]}" \
   --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_otb -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/otb-VERSION-ITERATION.centos7.ARCH.rpm usr

   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_otb
}
#-----------------------------------------------------------#
function build_dir_tree()
{
   ##set PATH env variable to /usr/bin to avoid
   # cmake finding /lib/cmake before /usr/lib/cmake
   # also include /usr/local/bin for fpm
   PATH="/usr/bin:/usr/local/bin"

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
##### PREPARE ENVIRONEMENT FOR BUILDING OTB ###
###########################################################

##create folder tree: build, install and rpm
build_dir_tree

###########################################################
#####  OTB build, install and RPM generation         ######
###########################################################
##retrieve OTB sources compile and install
compile_OTB_package

##create RPM package
build_OTB_RPM_Package
