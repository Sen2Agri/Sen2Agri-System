#! /bin/bash


#UNCOMMENT THE DEFINITIONS BELOW IF YO NEED OTHER VALUES

#parameters for BVInputVariableGeneration
#GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
SOLAR_ZENITH_ANGLE=24.360002
SENSOR_ZENITH_ANGLE=1.072074
# Computed as PHI_V - 180 - PHI_S
# if (rel_az < -180.0) then rel_az = rel_az + 360.0
# if (rel_az > 180.0) then rel_az = rel_az - 360.0
# PHI_V>-73.809703566</PHI_V>
#<PHI_S>145.43902353</PHI_S>
RELATIVE_AZIMUTH_ANGLE=-46.083528

#parameters for TrainingDataGenerator
#BV_IDX=0
#ADD_REFLS=0
#RED_INDEX=0
#NIR_INDEX=2

#parameters for generating model
#REGRESSION_TYPE="nn"
#BEST_OF=1

#end of USER modif

../../sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing.py --applocation /home/ramona/sen2agri-build --input \
"/mnt/output/L2A/Sentinel-2/Toulouse/31TCJ/S2A_OPER_SSC_L2VALD_31TCJ____20150706/S2A_OPER_SSC_L2VALD_31TCJ____20150706.HDR" \
--res 0 --t0 20150706 --tend 20150706 --outdir /mnt/data/S2_Results/LAI_Toulouse_Monodate --rsrfile /home/ramona/sen2agri/sen2agri-processors/VegetationStatus/otb-bv/data/Sentinel2.rsr --solarzenith $SOLAR_ZENITH_ANGLE --sensorzenith $SENSOR_ZENITH_ANGLE --relativeazimuth $RELATIVE_AZIMUTH_ANGLE
