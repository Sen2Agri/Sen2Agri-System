#! /bin/bash

MY_PWD=`pwd`

VEG_STATUS_BUILD="~/sen2agri-processors-build/VegetationStatus/"

#cd $MY_PWD

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_E/
./lai_retr_spot4_midp_E.sh $VEG_STATUS_BUILD 0 /mnt/data/output_temp/out_l3b_spot4_midp_E/

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_W/
./lai_retr_spot4_midp_W.sh $VEG_STATUS_BUILD 0 /mnt/data/output_temp/out_l3b_spot4_midp_W/

mkdir /mnt/data/output_temp/out_l3b_spot4_ukraine/
./lai_retr_spot4_ukraine.sh $VEG_STATUS_BUILD 0 /mnt/data/output_temp/out_l3b_spot4_ukraine/

mkdir /mnt/data/output_temp/out_l3b_spot4_south_africa/
./lai_retr_spot4_south_africa.sh $VEG_STATUS_BUILD 0 /mnt/data/output_temp/out_l3b_spot4_south_africa/
