#! /bin/bash

MY_PWD=`pwd`

echo "Starting tests ..." > "$MY_PWD/Tests_Results.txt"

function timedExec {
    TEST_NAME="$1"
    IN_PARAM="$2"
    time ($IN_PARAM) >> "$MY_PWD/Tests_Results.txt" 2>&1
    #mytime="$(time ($IN_PARAM))"
    #echo "Time for $TEST_NAME is : $mytime" >> "$MY_PWD/Tests_Results.txt"
}

cd $MY_PWD
cd Composite/clipping

mkdir /mnt/data/output_temp/out_l3a_spot4_l8_ukraine_clip/
./spot4_l8_ukraine.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_ukraine_clip/

#mkdir /mnt/data/output_temp/out_l3a_spot4_south_africa_clip/
#timedExec "out_l3a_spot4_south_africa_clip" "./spot4_south_africa.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_south_africa_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot4_spot4_ukraine_clip/
#timedExec "out_l3a_spot4_spot4_ukraine_clip" "./spot4_ukraine.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_spot4_ukraine_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot4_midp_E_clip/
#timedExec "out_l3a_spot4_midp_E_clip" "./midp_E.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_midp_E_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot4_midp_W_clip/
#timedExec "out_l3a_spot4_midp_W_clip" "./midp_W.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_midp_W_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot4_l8_midp_W_clip/
#timedExec "out_l3a_spot4_l8_midp_W_clip" "./spot4_l8_midp_W.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_midp_W_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot4_l8_south_africa_clip/
#timedExec "out_l3a_spot4_l8_south_africa_clip" "./spot4_l8_south_africa.sh ~/sen2agri-processors-build/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_south_africa_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot5_belgium_clip/
#timedExec "out_l3a_spot5_belgium_clip" "./spot5_belgium.sh ~/sen2agri-processors-build/ 10 /mnt/data/output_temp/out_l3a_spot5_belgium_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_spot5_ukraine_clip/
#timedExec "out_l3a_spot5_ukraine_clip" "./spot5_ukraine.sh ~/sen2agri-processors-build/ 10 /mnt/data/output_temp/out_l3a_spot5_ukraine_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_l8_belgium_clip/
#timedExec "out_l3a_l8_belgium_clip" "./l8_belgium.sh ~/sen2agri-processors-build/ 30 /mnt/data/output_temp/out_l3a_l8_belgium_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_l8_midp_W_clip/
#timedExec "out_l3a_l8_midp_W_clip" "./l8_midp_W.sh ~/sen2agri-processors-build/ 30 /mnt/data/output_temp/out_l3a_l8_midp_W_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_l8_south_africa_clip/
#timedExec "out_l3a_l8_south_africa_clip" "./l8_south_africa.sh ~/sen2agri-processors-build/ 30 /mnt/data/output_temp/out_l3a_l8_south_africa_clip/"
#
#mkdir /mnt/data/output_temp/out_l3a_l8_ukraine_clip/
#timedExec "out_l3a_l8_ukraine_clip" "./l8_ukraine.sh ~/sen2agri-processors-build/ 30 /mnt/data/output_temp/out_l3a_l8_ukraine_clip/"
#