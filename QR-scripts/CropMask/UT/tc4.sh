#! /bin/bash

#All tests for the CropMask modules

#Update the following variables to match the build folder and modules output folder
CROPMASK_OTB_LIBS_ROOT="~/sen2agri-build/CropMask"
OUT_FOLDER="/mnt/data/TU/CropMask/TestOut"
REFERENCE_FOLDER="/mnt/data/TU/CropMask/Reference"

echo "Running tc4-1"
./tc4-1.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-2"
./tc4-2.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-3"
./tc4-3.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-4"
./tc4-4.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-5"
./tc4-5.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-6"
./tc4-6.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-7"
./tc4-7.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-8"
./tc4-8.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc4-9"
./tc4-9.sh $CROPMASK_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER

