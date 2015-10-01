#! /bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: $0 <include file> <out folder name>"
  echo "The output directory should be given" 1>&2  
  exit
fi

function try {
    echo
    echo
    echo "$@"
    "$@"
    code=$?
    if [ $code -ne 0 ]
    then
        echo "$1 did not work: exit status $code"
        exit 1
    fi
}

function getReducedAngle() {
    DECIMAL=$1

    INTEGER=${DECIMAL%.*}
    FRACT=${DECIMAL#*.}

    echo "$INTEGER.${FRACT:0:1}"
}

OUT_FOLDER=$2

VEG_STATUS_OTB_LIBS_ROOT="~/sen2agri-processors-build/VegetationStatus"
IMG_INV_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/otb-bv/src/applications"
OUT_GENERATED_SAMPLE_FILE="$OUT_FOLDER/out_bv_dist_samples.txt"

# THESE ARE DEFAULT PARAMETERS - THEY WILL BE OVERWRITTEN BY THE PARAMETERS IN
# THE INCLUDE FILE

#parameters for BVInputVariableGeneration
GENERATED_SAMPLES_NO=100

#parameters for ProSailSimulator
SATELLITE_NAME="SPOT4"
SOLAR_ZENITH_ANGLE=25
SENSOR_ZENITH_ANGLE=25
RELATIVE_AZIMUTH_ANGLE=10

#parameters for TrainingDataGenerator
BV_IDX=0
ADD_REFLS=0
RED_INDEX=0
NIR_INDEX=2

#parameters for generating model
REGRESSION_TYPE="nn"
BEST_OF=1

#USER modif
#add directories where SPOT products are to be found
source $1
#end of USER modif

RSR_FILE=""
if [ "$SATELLITE_NAME" == "SPOT4" ]; then
    RSR_FILE="../otb-bv/data/spot4hrvir1.rsr"
else 
    if [ "$SATELLITE_NAME" == "SPOT5" ]; then
        RSR_FILE="../otb-bv/data/spot5hrg1.rsr"
    else
        if [ "$SATELLITE_NAME" == "S2" ]; then
            RSR_FILE="../otb-bv/data/Sentinel2.rsr"
        else
            if [ "$SATELLITE_NAME" == "L8" ]; then
                RSR_FILE="../otb-bv/data/landsat8.rsr"
            else
                echo "Unknown satellite name: $SATELLITE_NAME" 
            fi
        fi
    fi
fi

# Variables for Prosail Simulator

OUT_SIMU_REFLS_FILE="$OUT_FOLDER/out_simu_refls.txt"
OUT_TRAINING_FILE="$OUT_FOLDER/out_training.txt"

SOLAR_ZENITH_REDUCED=$( getReducedAngle $SOLAR_ZENITH_ANGLE)
SENSOR_ZENITH_REDUCED=$( getReducedAngle $SENSOR_ZENITH_ANGLE)
REL_AZIMUTH_REDUCED=$( getReducedAngle $RELATIVE_AZIMUTH_ANGLE)

echo "SOLAR ANGLE reduced from $SOLAR_ZENITH_ANGLE to $SOLAR_ZENITH_REDUCED"
echo "SENSOR ANGLE reduced from $SENSOR_ZENITH_ANGLE to $SENSOR_ZENITH_REDUCED"
echo "AZIMUTH ANGLE reduced from $RELATIVE_AZIMUTH_ANGLE to $REL_AZIMUTH_REDUCED"

# Variables for InverseModelLearning
OUT_MODEL_FILE="$OUT_FOLDER/Model_THETA_S_"$SOLAR_ZENITH_REDUCED"_THETA_V_"$SENSOR_ZENITH_REDUCED"_REL_PHI_"$REL_AZIMUTH_REDUCED".txt"
OUT_ERR_EST_FILE="$OUT_FOLDER/Err_Est_Model_THETA_S_"$SOLAR_ZENITH_REDUCED"_THETA_V_"$SENSOR_ZENITH_REDUCED"_REL_PHI_"$REL_AZIMUTH_REDUCED".txt"

PARAMS_TXT="$OUT_FOLDER/generate_lai_model_params.txt"
rm -fr $PARAMS_TXT
touch $PARAMS_TXT

echo "BVInputVariableGeneration" >> $PARAMS_TXT
echo "    Number of generated samples   = $GENERATED_SAMPLES_NO" >> $PARAMS_TXT
echo "ProSailSimulator" >> $PARAMS_TXT
echo "    RSR file                      = $RSR_FILE" >> $PARAMS_TXT
echo "    Solar zenith angle            = $SOLAR_ZENITH_ANGLE" >> $PARAMS_TXT
echo "    Sensor zenith angle           = $SENSOR_ZENITH_ANGLE" >> $PARAMS_TXT
echo "    Relative azimuth angle        = $RELATIVE_AZIMUTH_ANGLE" >> $PARAMS_TXT
echo "TrainingDataGenerator" >> $PARAMS_TXT
echo "    BV Index                      = $BV_IDX" >> $PARAMS_TXT
echo "    Add reflectances              = $ADD_REFLS" >> $PARAMS_TXT
echo "    RED Band Index                = $RED_INDEX" >> $PARAMS_TXT
echo "    NIR Band Index                = $NIR_INDEX" >> $PARAMS_TXT
echo "Inverse model generation (InverseModelLearning)" >> $PARAMS_TXT
echo "    Regression type               = $REGRESSION_TYPE" >> $PARAMS_TXT
echo "    Best of                       = $BEST_OF" >> $PARAMS_TXT
echo "    Generated model file name     = $OUT_MODEL_FILE" >> $PARAMS_TXT
echo "    Generated error estimation model file name = $OUT_ERR_EST_FILE" >> $PARAMS_TXT
echo " " >> $PARAMS_TXT

#generating Input BV Distribution file
echo "Generating Input BV Distribution file ..."
try otbcli BVInputVariableGeneration $IMG_INV_OTB_LIBS_ROOT -samples $GENERATED_SAMPLES_NO -out $OUT_GENERATED_SAMPLE_FILE

# Generating simulation reflectances
echo "Generating simulation reflectances ..."
try otbcli ProSailSimulator $IMG_INV_OTB_LIBS_ROOT -bvfile $OUT_GENERATED_SAMPLE_FILE -rsrfile $RSR_FILE -out $OUT_SIMU_REFLS_FILE -solarzenith $SOLAR_ZENITH_ANGLE -sensorzenith $SENSOR_ZENITH_ANGLE -azimuth $RELATIVE_AZIMUTH_ANGLE

# Generating training file
echo "Generating training file ..."
try otbcli TrainingDataGenerator $IMG_INV_OTB_LIBS_ROOT -biovarsfile $OUT_GENERATED_SAMPLE_FILE -simureflsfile $OUT_SIMU_REFLS_FILE -outtrainfile $OUT_TRAINING_FILE -bvidx $BV_IDX -addrefls $ADD_REFLS -redidx $RED_INDEX -niridx $NIR_INDEX

# Generating model
echo "Generating model ..."
try otbcli InverseModelLearning $IMG_INV_OTB_LIBS_ROOT -training $OUT_TRAINING_FILE -out $OUT_MODEL_FILE -errest $OUT_ERR_EST_FILE -regression $REGRESSION_TYPE -bestof $BEST_OF


