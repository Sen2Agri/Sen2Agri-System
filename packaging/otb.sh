#!/bin/sh
git clone https://github.com/GrayShade/OTB.git
sudo ln -s /usr/bin/opj2_compress /usr/bin/opj_compress
sudo ln -s /usr/bin/opj2_decompress /usr/bin/opj_decompress
sudo ln -s /usr/bin/opj2_dump /usr/bin/opj_dump

mkdir OTB-BUILD
cd OTB-BUILD
cmake ../OTB-5.2.0/SuperBuild \
	-DCMAKE_INSTALL_PREFIX=~/otb-install/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_TESTING=OFF \
	-DGDAL_CONFIG=/usr/local/bin/gdal-config \
	-DGDAL_INCLUDE_DIR=/usr/local/include \
	-DGDAL_LIBRARY=/usr/local/lib/libgdal.so \
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

sed -i -e "s|/home/grayshade/otb-install/|/|g" ~/otb-install/usr/lib/cmake/OTB-5.2/*.cmake
sed -i -e "s|/home/grayshade/otb-install/|/|g" ~/otb-install/usr/lib/cmake/OTB-5.2/Modules/*.cmake
sed -i -e "s|/home/grayshade/otb-install/|/|g" ~/otb-install/usr/lib/pkgconfig/openthreads.pc

mkdir ~/tmp
#fpm -s dir -t rpm -n otb -v 5.0 -C ~/otb-install/ --workdir ~/tmp -p otb-VERSION.fc22.ARCH.rpm usr
fpm -s dir -t rpm -n otb -v 5.2.0 -C ~/otb-install/ --workdir ~/tmp -p otb-VERSION.centos7.ARCH.rpm usr
