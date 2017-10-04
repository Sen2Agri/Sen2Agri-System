#!/bin/bash
set -e
cd sen2agri/packaging
rm -rf Sen2AgriPlatform \
       Sen2AgriApp \
       Sen2AgriProcessors \
       Sen2AgriWebSite
rm -rf Sen2AgriRPM/sen2agri-app-*.rpm \
       Sen2AgriRPM/sen2agri-downloaders-demmaccs-*.rpm \
       Sen2AgriRPM/sen2agri-processors-*.rpm \
       Sen2AgriRPM/sen2agri-website-*.rpm \

if ! ls Sen2AgriRPM/otb-*.rpm 2>&1 >/dev/null
then
    ./Sen2AgriPlatformBuild.sh
fi

sudo yum -y install Sen2AgriRPM/otb-*.rpm

./Sen2AgriProcessorsBuild.sh
./Sen2AgriAppBuild.sh
./Sen2AgriWebSiteBuild.sh

mkdir -p Sen2AgriRPM
mv Sen2AgriApp/rpm_binaries/*.rpm \
   Sen2AgriProcessors/rpm_binaries/*.rpm \
   Sen2AgriWebSite/rpm_binaries/*.rpm \
   Sen2AgriRPM

rm -rf Sen2AgriApp \
       Sen2AgriProcessors \
       Sen2AgriWebSite
