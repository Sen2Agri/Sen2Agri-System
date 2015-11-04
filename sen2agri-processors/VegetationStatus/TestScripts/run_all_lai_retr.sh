#! /bin/bash

MY_PWD=`pwd`

VEG_STATUS_BUILD="~/sen2agri-processors-build/VegetationStatus/"
RSR_FILE="~/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr"

#cd $MY_PWD

mkdir /mnt/data/output_temp/out_l3b_spot4_ukraine/
./lai_retr_spot4_ukraine.sh $VEG_STATUS_BUILD 0 $RSR_FILE /mnt/data/output_temp/out_l3b_spot4_ukraine/ $MY_PWD

mkdir /mnt/data/output_temp/out_l3b_spot4_south_africa/
./lai_retr_spot4_south_africa.sh $VEG_STATUS_BUILD 0 $RSR_FILE /mnt/data/output_temp/out_l3b_spot4_south_africa/ $MY_PWD

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_E/
./lai_retr_spot4_midp_E.sh $VEG_STATUS_BUILD 0 $RSR_FILE /mnt/data/output_temp/out_l3b_spot4_midp_E/ $MY_PWD

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_W/
./lai_retr_spot4_midp_W.sh $VEG_STATUS_BUILD 0 $RSR_FILE /mnt/data/output_temp/out_l3b_spot4_midp_W/ $MY_PWD

