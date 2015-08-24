#! /bin/sh

if [ $# -lt 2 ]
then
  echo "Usage: $0 <in spot4 xml fle> <out folder name>"
  exit
fi

INPUT_SPOT_XML_FILE=$1
OUT_FOLDER=$2

COMPOSITE_OTB_LIBS_ROOT="../../../sen2agri-processors-build/Composite"
WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"

OUT_SPOT_MASKS="$OUT_FOLDER/spot_masks.tif"
OUT_IMG_10M="$OUT_FOLDER/res10.tif"
OUT_IMG_20M="$OUT_FOLDER/res20.tif"
OUT_CLD_10M="$OUT_FOLDER/cld10.tif"
OUT_CLD_20M="$OUT_FOLDER/cld20.tif"
OUT_WAT_10M="$OUT_FOLDER/wat10.tif"
OUT_WAT_20M="$OUT_FOLDER/wat20.tif"
OUT_SNOW_10M="$OUT_FOLDER/snow10.tif"
OUT_SNOW_20M="$OUT_FOLDER/snow20.tif"
OUT_AOT_10M="$OUT_FOLDER/aot10.tif"
OUT_AOT_20M="$OUT_FOLDER/aot20.tif"


OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tiff"
OUT_WEIGHT_CLOUD_FILE="$OUT_FOLDER/WeightCloud.tiff"
OUT_TOTAL_WEIGHT_FILE="$OUT_FOLDER/WeightTotal.tiff"
OUT_L3A_FILE="$OUT_FOLDER/L3AResult.tiff"

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
L3A_DATE="20140502"
HALF_SYNTHESIS="50"
WEIGHT_DATE_MIN="0.10"

MY_PWD=`pwd`

#function try {
#    "$@"
#    code=$?
#    if [ $code -ne 0 ]
#    then
#        echo "$1 did not work: exit status $code"
#        exit 1
#    fi
#}
#if [[ $# -ne 2 ]]; then
#    echo "The xml file with full path for the SPOT product should be given. The output directory should be given" 1>&2
#    exit 1
#fi

otbcli SpotMaskHandler $COMPOSITE_OTB_LIBS_ROOT/SpotMaskHandler/ -xml $INPUT_SPOT_XML_FILE -out $OUT_SPOT_MASKS
otbcli ResampleAtS2Res $COMPOSITE_OTB_LIBS_ROOT/ResampleAtS2Res/ -xml $INPUT_SPOT_XML_FILE -spotmask $OUT_SPOT_MASKS -outres10 $OUT_IMG_10M -outres20 $OUT_IMG_20M -outcmres10 $OUT_CLD_10M -outwmres10 $OUT_WAT_10M -outsmres10 $OUT_SNOW_10M -outaotres10 $OUT_AOT_10M -outcmres20 $OUT_CLD_20M -outwmres20 $OUT_WAT_20M -outsmres20 $OUT_SNOW_20M -outaotres20 $OUT_AOT_20M 

echo "Executing from $MY_PWD"

otbcli WeightAOT $WEIGHT_OTB_LIBS_ROOT/WeightAOT/ -in $OUT_AOT_10M -xml $INPUT_SPOT_XML_FILE -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out $OUT_WEIGHT_AOT_FILE

otbcli WeightOnClouds $WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/ -incldmsk $OUT_CLD_10M -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out $OUT_WEIGHT_CLOUD_FILE

otbcli TotalWeight $WEIGHT_OTB_LIBS_ROOT/TotalWeight/ -xml $INPUT_SPOT_XML_FILE -waotfile $OUT_WEIGHT_AOT_FILE -wcldfile $OUT_WEIGHT_CLOUD_FILE -wsensor $WEIGHT_SENSOR -l3adate $L3A_DATE -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out $OUT_TOTAL_WEIGHT_FILE

otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_10M -res 10 -xml $INPUT_SPOT_XML_FILE -csm $OUT_CLD_10M -wm $OUT_WAT_10M -sm $OUT_SNOW_10M -wl2a $OUT_TOTAL_WEIGHT_FILE -out $OUT_L3A_FILE
