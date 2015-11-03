#!/bin/bash

./spot4_south_africa_windows.sh /home/agrosu/sen2agri-processors-build/Composite 20 /mnt/scratch/composite_southafrica bands_mapping_spot.txt
./spot4_ukraine_windows.sh /home/agrosu/sen2agri-processors-build/Composite 20 /mnt/scratch/composite_southafrica bands_mapping_spot.txt
./midp_W_windows.sh /home/agrosu/sen2agri-processors-build/Composite 20 /mnt/scratch/composite_southafrica bands_mapping_spot.txt
