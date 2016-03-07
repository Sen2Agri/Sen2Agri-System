#!/bin/bash
#set -x #echo on

##
## SCRIPT: BUILD SEN2AGRI PLATFORM OTB AND GDAL TOOLS
##
##
## SCRIPT STEPS
##     - CREATE DIR TREE: Sen2AgriPlatform/install, Sen2AgriPlatform/build and Sen2AgriPlatform/rpm_binaries
##     - RETRIEVE COMPILE AND INSTALL OTB APP
##     - RPM GENERATION FOR COMPILED OTB APP
##     - RETRIEVE COMPILE AND INSTALL GDAL APP
##     - RPM GENERATION FOR COMPILED GDAL APP
################################################################################################
###########################CONFIG PART###########################################################
### URLs FOR RETRIEVING SOURCES PACKAGES
: ${OTB_URL:="https://github.com/GrayShade/OTB.git"}
: ${OTB_MOSAIC_URL:="https://github.com/remicres/otb-mosaic.git"}
: ${GDAL_URL:="http://download.osgeo.org/gdal"}

### DEPENDENCIES FOR GENERATED RPM PACKAGES
: ${PLATFORM_INSTALL_DEP:="-d "boost" -d "curl" -d "expat" -d "fftw" -d "gdal" -d "geos" -d "libgeotiff" -d "libjpeg-turbo" -d "libsvm" -d "muParser" \
-d "opencv" -d "openjpeg2" -d "openjpeg2-tools" -d "pcre" -d "libpng" -d "proj" -d "python" -d "qt" -d "sqlite" -d "swig" -d "libtiff" -d "tinyxml" \
-d "qt5-qtbase" -d "qt5-qtbase-postgresql" -d "qt-x11" -d "gsl""}
: ${PLATFORM_INSTALL_CIFS_DEP:="-d "cifs-utils""}

### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_DIR:=$(pwd)}
: ${PLATFORM_NAME_DIR:="Sen2AgriPlatform"}
: ${INSTALL_DIR:="install"}
: ${RPM_DIR:="rpm_binaries"}
: ${BUILD_DIR:="build"}
: ${WORKING_DIR_INSTALL:=${PLATFORM_NAME_DIR}/${INSTALL_DIR}}
: ${WORKING_DIR_RPM:=${PLATFORM_NAME_DIR}/${RPM_DIR}}
: ${WORKING_DIR_BUILD:=${PLATFORM_NAME_DIR}/${BUILD_DIR}}
: ${GDAL_VERSION:="2.0.1"}
: ${OTB_VERSION:="5.0"}
: ${GDAL_INSTALL_PATH:="${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/gdal-install"}
: ${OTB_INSTALL_PATH:="${DEFAULT_DIR}/${WORKING_DIR_INSTALL}/otb-install"}
################################################################################################
#-----------------------------------------------------------#
function compile_OTB_package()
{
   ##download OTB
   git clone --depth=1 ${OTB_URL} ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}

   ##download MOSAIC
   git clone --depth=1 ${OTB_MOSAIC_URL} ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-MOSAIC

   ##copy MOSAIC into OTB/Modules/Remote
   mv ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-MOSAIC ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}/Modules/Remote
    
   ## go into OTB build dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}-BUILD
   cd ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/OTB-${OTB_VERSION}-BUILD
        
   ##REPLACE LIB LINKS before launching config
   sudo ln -s /usr/bin/opj2_decompress /usr/bin/opj_decompress
   sudo ln -s /usr/bin/opj2_compress /usr/bin/opj_compress
   sudo ln -s /usr/bin/opj2_dump /usr/bin/opj_dump
        
   ##configure OTB
   cmake ../OTB-${OTB_VERSION}/SuperBuild \
	 -DCMAKE_INSTALL_PREFIX=${OTB_INSTALL_PATH}/usr \
	 -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	 -DBUILD_TESTING=OFF \
	 -DGDAL_CONFIG=/usr/bin/gdal-config \
	 -DGDAL_INCLUDE_DIR=/usr/include/gdal \
	 -DGDAL_LIBRARY=/usr/lib64/libgdal.so \
	 -DOTB_USE_MUPARSERX=OFF \
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
	 -DUSE_SYSTEM_LIBKML=OFF \
	 -DUSE_SYSTEM_LIBSVM=ON \
	 -DUSE_SYSTEM_MUPARSER=ON \
	 -DUSE_SYSTEM_MUPARSERX=ON \
	 -DUSE_SYSTEM_OPENCV=ON \
	 -DUSE_SYSTEM_OPENJPEG=ON \
	 -DUSE_SYSTEM_OPENTHREADS=OFF \
	 -DUSE_SYSTEM_OSSIM=OFF \
	 -DUSE_SYSTEM_PCRE=ON \
	 -DUSE_SYSTEM_PNG=ON \
	 -DUSE_SYSTEM_PROJ=ON \
	 -DUSE_SYSTEM_QT4=ON \
	 -DUSE_SYSTEM_SQLITE=ON \
	 -DUSE_SYSTEM_SWIG=ON \
	 -DUSE_SYSTEM_TIFF=ON \
	 -DUSE_SYSTEM_TINYXML=ON \
	 -DUSE_SYSTEM_ZLIB=ON

   ## compile OTB
   make
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
   fpm -s dir -t rpm -n otb -v ${OTB_VERSION} -C ${OTB_INSTALL_PATH}/ ${PLATFORM_INSTALL_DEP} ${PLATFORM_INSTALL_CIFS_DEP} \
   --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_otb -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/otb-VERSION.centos7.ARCH.rpm usr
  
   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_otb
}
#-----------------------------------------------------------#
function compile_GDAL_package()
{
   ## Compiling GDAL 2.0

   #Some processors use the GDAL tools, which are significantly faster in version 2.0. We'll install that in `/usr/local`.
  
   cd ${DEFAULT_DIR}/${WORKING_DIR_BUILD} && { curl -O ${GDAL_URL}/${GDAL_VERSION}/gdal-${GDAL_VERSION}.tar.gz ; cd -; }
   
   ## Decompress the archieve
   tar -zxvf ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/gdal-${GDAL_VERSION}.tar.gz -C ${DEFAULT_DIR}/${WORKING_DIR_BUILD}
   cd ${DEFAULT_DIR}/${WORKING_DIR_BUILD}/gdal-${GDAL_VERSION}

   ## Configure, compile and install
   ./configure
   make -j12
   make
   make install DESTDIR=${GDAL_INSTALL_PATH}
}
#-----------------------------------------------------------#
function build_GDAL_RPM_Package()
{
   ## create a temp working dir
   mkdir -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_gdal

   ##    ## build the packages specifying installing dependencies
   fpm -s dir -t rpm -n gdal-local -v ${GDAL_VERSION} ${PLATFORM_INSTALL_DEP} ${PLATFORM_INSTALL_CIFS_DEP} -C ${GDAL_INSTALL_PATH} \
   -p ${DEFAULT_DIR}/${WORKING_DIR_RPM}/gdal-local-VERSION.centos7.ARCH.rpm --workdir ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_gdal usr
   
   #remove temporary dir
   rm -rf ${DEFAULT_DIR}/${WORKING_DIR_RPM}/tmp_gdal
   
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
   
   ##create build dir
   if [ ! -d ${BUILD_DIR} ]; then
      mkdir -p ${BUILD_DIR}
   fi
   
   ##exit from platform dir
   cd ..
}

###########################################################
##### PREPARE ENVIRONEMENT FOR BUILDING OTB AND GDAL  ###
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
###########################################################
#####  GDAL build, install and RPM generation         ##### 
###########################################################
##retrieve GDAL sources compile and install
compile_GDAL_package

##create RPM package
build_GDAL_RPM_Package
