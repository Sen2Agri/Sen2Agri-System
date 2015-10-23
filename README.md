# Prerequisites

    yum install epel-release

    yum install cmake
    yum install boost-devel curl-devel expat-devel fftw-devel gdal-devel geos-devel libgeotiff-devel itk-devel libjpeg-turbo-devel libsvm-devel muParser-devel opencv-devel openjpeg-devel pcre-devel libpng-devel proj-devel qt4 sqlite-devel swig libtiff-devel tinyxml-devel qt5-qtbase-devel qt5-qtbase-postgresql

# Compiling OTB

    git clone https://github.com/GrayShade/OTB.git # fixes-5.0 branch
    mkdir OTB-BUILD
    cd OTB-BUILD
    ccmake ../OTB/SuperBuild

    Press 'c' (configure), 'e' (exit the warning screen), 't' (toggle advanced mode)

    Change:

    CMAKE_BUILD_TYPE        RelWithDebInfo

    OTB_USE_MUPARSERX       OFF
    OTB_WRAP_PYTHON         ON

    USE_SYSTEM_BOOST        ON
    USE_SYSTEM_CURL         ON
    USE_SYSTEM_EXPAT        ON
    USE_SYSTEM_FFTW         ON
    USE_SYSTEM_GDAL         ON
    USE_SYSTEM_GEOS         ON
    USE_SYSTEM_GEOTIFF      ON
    USE_SYSTEM_ITK          OFF
    USE_SYSTEM_JPEG         ON
    USE_SYSTEM_LIBKML       OFF
    USE_SYSTEM_LIBSVM       ON
    USE_SYSTEM_MUPARSER     ON
    USE_SYSTEM_MUPARSERX    ON
    USE_SYSTEM_OPENCV       ON
    USE_SYSTEM_OPENJPEG     ON
    USE_SYSTEM_OPENTHREADS  OFF
    USE_SYSTEM_OSSIM        OFF
    USE_SYSTEM_PCRE         ON
    USE_SYSTEM_PNG          ON
    USE_SYSTEM_PROJ         ON
    USE_SYSTEM_QT4          ON
    USE_SYSTEM_SQLITE       ON
    USE_SYSTEM_SWIG         ON
    USE_SYSTEM_TIFF         ON
    USE_SYSTEM_TINYXML      ON
    USE_SYSTEM_XLIB         ON

    Press 'c', 'e', 'c', 'e'. Ignore the OpenThreads warning. Press 'g' (generate and exit).

    sudo make

