#!/bin/bash

./spot4_ukraine_windows.sh ~/sen2agri-processors-build 20 /mnt/data/output_temp/composite_ukraine
./spot4_south_africa_windows.sh ~/sen2agri-processors-build 20 /mnt/data/output_temp/composite_south_africa
./midp_W_windows.sh ~/sen2agri-processors-build 20 /mnt/data/output_temp/composite_sudmidp_W
