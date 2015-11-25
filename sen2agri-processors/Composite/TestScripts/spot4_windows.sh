#!/bin/bash

./spot4_south_africa_windows.sh ~/sen2agri-processors-build/ 20 /mnt/scratch/composite_alex_southafrica bands_mapping_spot.txt
./spot4_ukraine_windows.sh ~/sen2agri-processors-build/ 20 /mnt/scratch/composite_alex_ukraine bands_mapping_spot.txt
./midp_W_windows.sh ~/sen2agri-processors-build/ 20 /mnt/scratch/composite_alex_midp_W bands_mapping_spot.txt
