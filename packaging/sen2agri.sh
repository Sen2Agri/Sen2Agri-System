#/bin/sh
cmake ~/sen2agri/sen2agri-processors -DCMAKE_INSTALL_PREFIX=~/sen2agri-processors-install/usr -DCMAKE_BUILD_TYPE=Release
mkdir ~/sen2agri-processors-install
make install
cd ..
mkdir tmp
fpm -s dir -t rpm -n sen2agri-processors -v 0.8 -C ~/sen2agri-processors-install -p sen2agri-processors-VERSION.centos7.ARCH.rpm --workdir ~/tmp usr
