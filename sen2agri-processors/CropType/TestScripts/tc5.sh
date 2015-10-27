#! /bin/bash

#All tests for the CropType modules

#Update the following variables to match the build folder and modules output folder
CROPTYPE_OTB_LIBS_ROOT="~/sen2agri-build/CropType"
OUT_FOLDER="/mnt/data/TU/CropType/TestOut"
REFERENCE_FOLDER="/mnt/data/TU/CropType/Reference"

echo "Running tc5-1"
./tc5-1.sh $CROPTYPE_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc5-2"
./tc5-2.sh $CROPTYPE_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc5-3"
./tc5-3.sh $CROPTYPE_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER
echo
echo "Running tc5-4"
./tc5-4.sh $CROPTYPE_OTB_LIBS_ROOT $REFERENCE_FOLDER $OUT_FOLDER

