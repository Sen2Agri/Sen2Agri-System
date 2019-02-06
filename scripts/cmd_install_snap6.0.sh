#!/usr/bin/bash
cd /opt

sudo wget http://step.esa.int/downloads/6.0/installers/esa-snap_sentinel_unix_6_0.sh && \
sudo chmod +x esa-snap_sentinel_unix_6_0.sh && \
sudo ./esa-snap_sentinel_unix_6_0.sh -q && \
sudo /opt/snap/bin/snap --nosplash --nogui --modules --update-all
sudo rm -f esa-snap_sentinel_unix_6_0.sh


LD_LIBRARY_PATH=/opt/snap/jre/lib/

### check if symbolik link; or create it
if [ ! -h /usr/local/bin/gpt ]; then sudo ln -s /opt/snap/bin/gpt /usr/local/bin/gpt;fi


echo All done! Let's check "gpt -h"
gpt -h

