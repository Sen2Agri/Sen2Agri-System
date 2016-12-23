#! /bin/bash

#UNCOMMENT THE DEFINITIONS BELOW IF YO NEED OTHER VALUES

#parameters for BVInputVariableGeneration
#GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
#SOLAR_ZENITH_ANGLE=70.733494344
#SENSOR_ZENITH_ANGLE=18.141708167
# Computed as PHI_V - 180 - PHI_S
# if (rel_az < -180.0) then rel_az = rel_az + 360.0
# if (rel_az > 180.0) then rel_az = rel_az - 360.0
# PHI_V>-73.809703566</PHI_V>
#<PHI_S>145.43902353</PHI_S>
#RELATIVE_AZIMUTH_ANGLE=-44.00829505

#parameters for TrainingDataGenerator
#BV_IDX=0
#ADD_REFLS=0
#RED_INDEX=0
#NIR_INDEX=2

#parameters for generating model
#REGRESSION_TYPE="nn"
#BEST_OF=1

CUR_DATE=`date +%Y-%m-%d`

#end of USER modif

./lai_retrieve_processing.py --applocation ~/sen2agri-processors-build --input \
"/mnt/Sen2Agri_DataSets/2013/Ukraine/EO/L8/MACCS_Format/EUkraineS2A_20130416_L8_181_025/EUkraineS2A_20130416_L8_181_025.HDR" \
--res 0 --outdir /mnt/output/L3B/SPOT4-T5/Ukraine/$CUR_DATE --rsrfile /usr/share/sen2agri/spot4hrvir1.rsr --modelsfolder /mnt/output/L3B/SPOT4-T5/GeneratedModels/ --generatemodel YES --genreprocessedlai NO --genfittedlai NO



