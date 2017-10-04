FROM centos:centos7

LABEL name="sen2agri-build" \
      maintainer="Laurentiu Nicola <laurentiu.nicola@c-s.ro>"

ARG user
ARG group
ARG uid
ARG gid

RUN yum -y install https://yum.postgresql.org/9.4/redhat/rhel-7.3-x86_64/pgdg-centos94-9.4-3.noarch.rpm && \
    yum -y update && \
    yum -y install epel-release && \
    yum -y update && \
    yum -y install boost-devel \
                   cmake \
                   curl-devel \
                   expat-devel \
                   fftw-devel \
                   gcc \
                   gcc-c++ \
                   gdal-devel \
                   geos-devel \
                   gsl-devel \
                   libgeotiff-devel \
                   libjpeg-turbo-devel \
                   libpng-devel \
                   libsvm-devel \
                   libtiff-devel \
                   make \
                   muParser-devel \
                   opencv-devel \
                   openjpeg2-devel \
                   openjpeg2-tools \
                   pcre-devel \
                   proj-devel \
                   proj-epsg \
                   python-devel \
                   qt-devel \
                   qt-x11 \
                   qt5-qtbase-devel \
                   qt5-qtbase-postgresql \
                   rpm-build \
                   ruby-devel \
                   sqlite-devel \
                   sudo \
                   swig \
                   tinyxml-devel && \
    gem install fpm

RUN ln -s /usr/bin/opj2_decompress /usr/bin/opj_decompress && \
    ln -s /usr/bin/opj2_compress /usr/bin/opj_compress && \
    ln -s /usr/bin/opj2_dump /usr/bin/opj_dump

RUN groupadd -g $gid $group && \
    useradd -u $uid -g $gid -G wheel $user && \
    echo "%wheel        ALL=(ALL)       NOPASSWD: ALL" > /etc/sudoers.d/wheel

COPY entry.sh /
