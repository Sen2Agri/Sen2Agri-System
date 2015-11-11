# Prerequisites

    yum install epel-release
    yum install git gcc gcc-c++ cmake boost-devel curl-devel expat-devel fftw-devel gdal-devel geos-devel libgeotiff-devel libjpeg-turbo-devel libsvm-devel muParser-devel opencv-devel openjpeg2-devel openjpeg2-tools pcre-devel libpng-devel proj-devel python-devel qt-devel sqlite-devel swig libtiff-devel tinyxml-devel qt5-qtbase-devel qt5-qtbase-postgresql gsl-devel

# Compiling OTB

    git clone --depth=1 https://github.com/GrayShade/OTB.git # fixes-5.0 branch
    mkdir OTB-BUILD
    cd OTB-BUILD
    ccmake ../OTB/SuperBuild

Press `c` (configure), ignore the warnings, press `e` (exit the warning screen), `t` (toggle advanced mode). Change:

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
    USE_SYSTEM_ZLIB         ON

Press `c`, `e`, `c`. Ignore the OpenThreads warning. Press `e`, `g` (generate and exit).

    sudo make

# Adding OTB to the loader path

    echo /usr/local/lib | sudo tee /etc/ld.so.conf.d/local.conf
    sudo ldconfig

# Compiling GDAL 2.0

Some processors use the GDAL tools, which are significantly faster in version 2.0. We'll install that in `/usr/local`.

    curl -O http://download.osgeo.org/gdal/2.0.1/gdal-2.0.1.tar.gz
    tar zxvf gdal-2.0.1.tar.gz
    cd gdal-2.0.1
    ./configure
    make
    sudo make install

# Compiling the processors

Retrieve the source code (not available online) and place it in the `sen2agri` directory.

    mkdir sen2agri-processors-build
    cd sen2agri-processors-build
    cmake ../sen2agri/sen2agri-processors -DCMAKE_BUILD_TYPE=RelWithDebInfo
    make
    sudo make install

# Compiling the rest of the system

    mkdir sen2agri-build
    cd sen2agri-build
    qmake-qt5 ../sen2agri
    make
    sudo make install

