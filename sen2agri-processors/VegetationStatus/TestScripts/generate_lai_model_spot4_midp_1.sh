#! /bin/bash

if [ $# -lt 1 ]
then
  echo "Usage: $0 <out folder name>"
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

OUT_FOLDER=$1

VEG_STATUS_OTB_LIBS_ROOT="~/sen2agri-processors-build/VegetationStatus"
IMG_INV_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/otb-bv/src/applications"
OUT_GENERATED_SAMPLE_FILE="$OUT_FOLDER/out_bv_dist_samples.txt"

# Variables for Prosail Simulator
RSR_FILE="../otb-bv/data/spot4hrvir1.rsr"
OUT_SIMU_REFLS_FILE="$OUT_FOLDER/out_simu_refls.txt"
OUT_TRAINING_FILE="$OUT_FOLDER/out_training.txt"

# Variables for InverseModelLearning
OUT_MODEL_FILE="$OUT_FOLDER/out_model.txt"
OUT_ERR_EST_FILE="$OUT_FOLDER/out_model_errest.txt"

#generating Input BV Distribution file
echo "Generating Input BV Distribution file ..."
try otbcli BVInputVariableGeneration $IMG_INV_OTB_LIBS_ROOT -samples 100 -out $OUT_GENERATED_SAMPLE_FILE

# Generating simulation reflectances
echo "Generating simulation reflectances ..."
try otbcli ProSailSimulator $IMG_INV_OTB_LIBS_ROOT -bvfile $OUT_GENERATED_SAMPLE_FILE -rsrfile $RSR_FILE -out $OUT_SIMU_REFLS_FILE -solarzenith 63.9335336048 -sensorzenith 11.7371647127 -azimuth -40.2681487255

# Generating training file
echo "Generating training file ..."
try otbcli TrainingDataGenerator $IMG_INV_OTB_LIBS_ROOT -biovarsfile $OUT_GENERATED_SAMPLE_FILE -simureflsfile $OUT_SIMU_REFLS_FILE -outtrainfile $OUT_TRAINING_FILE -bvidx 0 -addrefls 0 -redidx 0 -niridx 2

# Generating model
echo "Generating model ..."
try otbcli InverseModelLearning $IMG_INV_OTB_LIBS_ROOT -training $OUT_TRAINING_FILE -out $OUT_MODEL_FILE -errest $OUT_ERR_EST_FILE -regression "nn" -bestof 1
