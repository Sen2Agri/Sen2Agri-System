#! /bin/bash

#./lai_retrieve_processing.py --applocation /home/agrosu/sen2agri-processors-build --input \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml" \
#--res 0 --outdir /mnt/scratch/lai_retr_python/python --rsrfile /home/agrosu/sen2agri/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr --solarzenith $SOLAR_ZENITH_ANGLE --sensorzenith $SENSOR_ZENITH_ANGLE --relativeazimuth $RELATIVE_AZIMUTH_ANGLE

#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130610_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130610_N2A_ESouthAfricaD0000B0000.xml "
#inputXML+="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130615_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130615_N2A_ESouthAfricaD0000B0000.xml "

#UNCOMMENT THE DEFINITIONS BELOW IF YO NEED OTHER VALUES

#parameters for BVInputVariableGeneration
#GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
SOLAR_ZENITH_ANGLE=67.372200673
SENSOR_ZENITH_ANGLE=0.23422731138
# Computed as PHI_V - 180 - PHI_S
# if (rel_az < -180.0) then rel_az = rel_az + 360.0
# if (rel_az > 180.0) then rel_az = rel_az - 360.0
# PHI_V>-73.809703566</PHI_V>
#<PHI_S>145.43902353</PHI_S>
RELATIVE_AZIMUTH_ANGLE=133.750784413

#parameters for TrainingDataGenerator
#BV_IDX=0
#ADD_REFLS=0
#RED_INDEX=0
#NIR_INDEX=2

#parameters for generating model
#REGRESSION_TYPE="nn"
#BEST_OF=1

#end of USER modif

#./lai_retrieve_processing.py --applocation /home/agrosu/sen2agri-processors-build --input \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml" \
#"/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml" \
#--res 0 --t0 20130131 --tend 20130225 --outdir /mnt/scratch/lai_retr_python/python --rsrfile /home/agrosu/sen2agri/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr --solarzenith $SOLAR_ZENITH_ANGLE --sensorzenith $SENSOR_ZENITH_ANGLE --relativeazimuth $RELATIVE_AZIMUTH_ANGLE
#exit 0
if [ $# -lt 5 ]
then
  echo "Usage: $0 <otb application directory> <resolution> <RSR filename> <out folder name> <scripts folder name> [test number]"
  echo "The resolution for which the computations will be performed should be given.  The RSR filename should be given. The output directory should be given" 1>&2  
  exit
fi

if [ $# == 5 ] ; then
"$5/run_lai_retrieve.sh" "$1" "$inputXML" "$2" "$4" "$3" $SOLAR_ZENITH_ANGLE $SENSOR_ZENITH_ANGLE $RELATIVE_AZIMUTH_ANGLE "$5"
else
"$5/run_lai_retrieve.sh" "$1" "$inputXML" "$2" "$4" "$3" $SOLAR_ZENITH_ANGLE $SENSOR_ZENITH_ANGLE $RELATIVE_AZIMUTH_ANGLE "$5" "$6"
fi

