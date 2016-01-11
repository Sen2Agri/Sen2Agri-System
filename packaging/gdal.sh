#!/bin/sh
#rm -rf gdal-2.0.1
#rm -rf ~/gdal-install

#tar zxf gdal-2.0.1.tar.gz
cd gdal-2.0.1
./configure
make -j12
mkdir ~/gdal-install
make install DESTDIR=~/gdal-install
cd ..
mkdir tmp
#fpm -s dir -t rpm -n gdal-local -v 2.0.1 -C ~/gdal-install -p gdal-local-VERSION.fc22.ARCH.rpm --workdir ~/tmp usr
fpm -s dir -t rpm -n gdal-local -v 2.0.1 -C ~/gdal-install -p gdal-local-VERSION.centos7.ARCH.rpm --workdir ~/tmp usr
