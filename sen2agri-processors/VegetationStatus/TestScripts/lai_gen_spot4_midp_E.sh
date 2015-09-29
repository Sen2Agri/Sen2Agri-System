#USER modif

#UNCOMMENT THE DEFINITIONS BELOW IF YO NEED OTHER VALUES

#parameters for BVInputVariableGeneration
#GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
SATELLITE_NAME="SPOT4"
SOLAR_ZENITH_ANGLE=65.4794212596
SENSOR_ZENITH_ANGLE=13.1300856884
# Computed as PHI_V - 180 - PHI_S
# if (rel_az < -180.0) then rel_az = rel_az + 360.0
# if (rel_az > 180.0) then rel_az = rel_az - 360.0
RELATIVE_AZIMUTH_ANGLE=140.964187309

#parameters for TrainingDataGenerator
#BV_IDX=0
#ADD_REFLS=0
#RED_INDEX=0
#NIR_INDEX=2

#parameters for generating model
#REGRESSION_TYPE="nn"
#BEST_OF=1

#end of USER modif