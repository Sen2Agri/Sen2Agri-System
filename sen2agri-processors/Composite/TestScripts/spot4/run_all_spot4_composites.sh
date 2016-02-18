#! /bin/bash

MY_PWD=`pwd`

echo "Starting tests ..." > "$MY_PWD/Tests_Results.txt"

function timed_exec {
    IN_PARAM="$1"
    echo "EXECUTING: $IN_PARAM" >> "$OUT_FOLDER/Execution_Times.txt"
    start=`date +%s`
    ($IN_PARAM)
    end=`date +%s`

    execution_time=$((end-start))
    echo -e "\nExecution took: $execution_time seconds\n\n" >> "$OUT_FOLDER/Execution_Times.txt"
}

ROOT_BUILD="~/sen2agri-processors-build/"
COMPOSITE_BUILD="$ROOT_BUILD/Composite/"

mkdir -p /mnt/data/output_temp/out_l3a_spot4_ukraine_ciu/
timed_exec "./spot4_ukraine.sh $COMPOSITE_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_ukraine_ciu/ bands_mapping_spot.txt"

#mkdir -p /mnt/data/output_temp/out_l3a_spot4_south_africa_ciu/
#timed_exec "./spot4_south_africa.sh $COMPOSITE_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_south_africa_ciu/ bands_mapping_spot.txt"
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_midp_W_ciu/
#timed_exec "./spot4_midp_W.sh $COMPOSITE_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_midp_W_ciu/ bands_mapping_spot.txt"

#mkdir -p /mnt/data/output_temp/out_l3a_spot4_midp_E_ciu/
#timed_exec "./midp_E.sh $COMPOSITE_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_midp_E_ciu/ bands_mapping_spot.txt"



#cd clipping
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_ukraine_clip_ciu/
#timed_exec "./spot4_ukraine.sh $ROOT_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_ukraine_clip_ciu/ bands_mapping_spot.txt"
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_south_africa_clip_ciu/
#timed_exec "./spot4_south_africa.sh $ROOT_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_south_africa_clip_ciu/ bands_mapping_spot.txt"
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_midp_W_clip_ciu/
#timed_exec "./spot4_midp_W.sh $ROOT_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_midp_W_clip_ciu/ bands_mapping_spot.txt"

#mkdir -p /mnt/data/output_temp/out_l3a_spot4_midp_E_clip_ciu/
#timed_exec "./midp_E.sh $ROOT_BUILD 20 /mnt/data/output_temp/out_l3a_spot4_midp_E_clip_ciu/ bands_mapping_spot.txt"


#mkdir -p /mnt/data/output_temp/out_l3a_spot4_l8_midp_W/
#./spot4_l8_midp_W.sh ~/sen2agri-processors-build/Composite/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_midp_W/
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_l8_south_africa/
#./spot4_l8_south_africa.sh ~/sen2agri-processors-build/Composite/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_south_africa/
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot4_l8_ukraine/
#./spot4_l8_ukraine.sh ~/sen2agri-processors-build/Composite/ 20 /mnt/data/output_temp/out_l3a_spot4_l8_ukraine/
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot5_belgium/
#./spot5_belgium.sh ~/sen2agri-processors-build/Composite/ 10 /mnt/data/output_temp/out_l3a_spot5_belgium/
#
#mkdir -p /mnt/data/output_temp/out_l3a_spot5_ukraine/
#./spot5_ukraine.sh ~/sen2agri-processors-build/Composite/ 10 /mnt/data/output_temp/out_l3a_spot5_ukraine/
#
#mkdir -p /mnt/data/output_temp/out_l3a_l8_belgium/
#./l8_belgium.sh ~/sen2agri-processors-build/Composite/ 30 /mnt/data/output_temp/out_l3a_l8_belgium/
#
#mkdir -p /mnt/data/output_temp/out_l3a_l8_midp_W/
#./l8_midp_W.sh ~/sen2agri-processors-build/Composite/ 30 /mnt/data/output_temp/out_l3a_l8_midp_W/
#
#mkdir -p /mnt/data/output_temp/out_l3a_l8_south_africa/
#./l8_south_africa.sh ~/sen2agri-processors-build/Composite/ 30 /mnt/data/output_temp/out_l3a_l8_south_africa/
#
#mkdir -p /mnt/data/output_temp/out_l3a_l8_ukraine/
#./l8_ukraine.sh ~/sen2agri-processors-build/Composite/ 30 /mnt/data/output_temp/out_l3a_l8_ukraine/
