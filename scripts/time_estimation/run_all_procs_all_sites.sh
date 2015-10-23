#! /bin/bash

MY_PWD=`pwd`

cd Composite
mkdir /mnt/data/output_temp/out_l3a_spot4_south_africa/
time ./spot4_south_africa.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_south_africa/

mkdir /mnt/data/output_temp/out_l3a_spot4_spot4_ukraine/
time ./spot4_ukraine.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_spot4_ukraine/

mkdir /mnt/data/output_temp/out_l3a_spot4_midp_E/
time ./midp_E.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_midp_E/

mkdir /mnt/data/output_temp/out_l3a_spot4_midp_W/
time ./midp_W.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_midp_W/

mkdir /mnt/data/output_temp/out_l3a_spot4_l8_midp_W/
time ./spot4_l8_midp_W.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_l8_midp_W/

mkdir /mnt/data/output_temp/out_l3a_spot4_l8_south_africa/
time ./spot4_l8_south_africa.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_l8_south_africa/

mkdir /mnt/data/output_temp/out_l3a_spot4_l8_ukraine/
time ./spot4_l8_ukraine.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot4_l8_ukraine/

mkdir /mnt/data/output_temp/out_l3a_spot5_belgium/
time ./spot5_belgium.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot5_belgium/

mkdir /mnt/data/output_temp/out_l3a_spot5_ukraine/
time ./spot5_ukraine.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_spot5_ukraine/

mkdir /mnt/data/output_temp/out_l3a_l8_belgium/
time ./l8_belgium.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_l8_belgium/

mkdir /mnt/data/output_temp/out_l3a_l8_midp_W/
time ./l8_midp_W.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_l8_midp_W/

mkdir /mnt/data/output_temp/out_l3a_l8_south_africa/
time ./l8_south_africa.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_l8_south_africa/

mkdir /mnt/data/output_temp/out_l3a_l8_ukraine/
time ./l8_ukraine.sh ~/sen2agri-processors-build/Composite/ 0 /mnt/data/output_temp/out_l3a_l8_ukraine/

cd $MY_PWD
cd VegetationStatus
mkdir /mnt/data/output_temp/out_l3b_spot4_belgium/
time ./lai_retr_spot4_belgium.sh 0 /mnt/data/output_temp/out_l3b_spot4_belgium/

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_E/
time ./lai_retr_spot4_midp_E.sh 0 /mnt/data/output_temp/out_l3b_spot4_midp_E/

mkdir /mnt/data/output_temp/out_l3b_spot4_midp_W/
time ./lai_retr_spot4_midp_W.sh 0 /mnt/data/output_temp/out_l3b_spot4_midp_W/

mkdir /mnt/data/output_temp/out_l3b_spot4_ukraine/
time ./lai_retr_spot4_ukraine.sh 0 /mnt/data/output_temp/out_l3b_spot4_ukraine/

mkdir /mnt/data/output_temp/out_l3b_spot4_south_africa/
time ./lai_retr_spot4_south_africa.sh 0 /mnt/data/output_temp/out_l3b_spot4_south_africa/


cd $MY_PWD
