#!/bin/bash
set -e

export LD_LIBRARY_PATH=/opt/rh/rh-ruby23/root/usr/lib64

cd sen2agri/packaging
rm -rf Sen2AgriPlatform \
       Sen2AgriApp \
       Sen2AgriProcessors \
       Sen2AgriWebSite
rm -rf Sen2AgriRPM/sen2agri-app-*.rpm \
       Sen2AgriRPM/sen2agri-downloaders-demmaccs-*.rpm \
       Sen2AgriRPM/sen2agri-processors-*.rpm \
       Sen2AgriRPM/sen2agri-website-*.rpm

mkdir -p Sen2AgriRPM
if test -z "$(shopt -s nullglob; echo Sen2AgriRPM/otb-*.rpm)"
then
    if test -z "$(shopt -s nullglob; echo Sen2AgriPlatform/rpm_binaries/otb-*.rpm)"
    then
        ./Sen2AgriPlatformBuild.sh
    fi
    cp Sen2AgriPlatform/rpm_binaries/otb-*.rpm Sen2AgriRPM
fi

sudo yum -y install Sen2AgriRPM/otb-*.rpm

./Sen2AgriProcessorsBuild.sh
./Sen2AgriAppBuild.sh
./Sen2AgriWebSiteBuild.sh

mv Sen2AgriApp/rpm_binaries/*.rpm \
   Sen2AgriProcessors/rpm_binaries/*.rpm \
   Sen2AgriWebSite/rpm_binaries/*.rpm \
   Sen2AgriRPM

rm -rf Sen2AgriApp \
       Sen2AgriProcessors \
       Sen2AgriWebSite
