#! /bin/bash

#UNCOMMENT THE DEFINITIONS BELOW IF YO NEED OTHER VALUES

#parameters for BVInputVariableGeneration
#GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
#SOLAR_ZENITH_ANGLE=63.9335336048
#SENSOR_ZENITH_ANGLE=11.7371647127
# Computed as PHI_V - 180 - PHI_S
# if (rel_az < -180.0) then rel_az = rel_az + 360.0
# if (rel_az > 180.0) then rel_az = rel_az - 360.0
# PHI_V>-73.809703566</PHI_V>
#<PHI_S>145.43902353</PHI_S>
#RELATIVE_AZIMUTH_ANGLE=-40.2681487255

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
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Sudmipy-West/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000.xml" \
--res 0 --outdir /mnt/output/L3B/SPOT4-T5/Midp_W/$CUR_DATE --rsrcfg /usr/share/sen2agri/rsr_cfg.txt --modelsfolder /mnt/output/L3B/SPOT4-T5/GeneratedModels/ --generatemodel YES --genreprocessedlai YES --genfittedlai YES
