#! /bin/sh

if [ $# -lt 1 ]
then
  echo "Usage: $0 <in spot4 xml fle> <out folder name>"
  echo "The xml file with full path for the SPOT product should be given. The output directory should be given" 1>&2  
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

inputXML[0]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml'
inputXML[1]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000.xml'
inputXML[2]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000.xml'
inputXML[3]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000.xml'
OUT_FOLDER=$1

COMPOSITE_OTB_LIBS_ROOT="~/sen2agri-processors-build/Composite"

WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"

OUT_SPOT_MASKS="$OUT_FOLDER/spot_masks.tif"
OUT_IMG_10M="$OUT_FOLDER/res10.tif"
OUT_IMG_10M_ALL="$OUT_FOLDER/res10_all.tif"
OUT_IMG_20M="$OUT_FOLDER/res20.tif"
OUT_IMG_20M_ALL="$OUT_FOLDER/res20_all.tif"
OUT_CLD_10M="$OUT_FOLDER/cld10.tif"
OUT_CLD_20M="$OUT_FOLDER/cld20.tif"
OUT_WAT_10M="$OUT_FOLDER/wat10.tif"
OUT_WAT_20M="$OUT_FOLDER/wat20.tif"
OUT_SNOW_10M="$OUT_FOLDER/snow10.tif"
OUT_SNOW_20M="$OUT_FOLDER/snow20.tif"
OUT_AOT_10M="$OUT_FOLDER/aot10.tif"
OUT_AOT_20M="$OUT_FOLDER/aot20.tif"


OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tif"
OUT_WEIGHT_CLOUD_FILE="$OUT_FOLDER/WeightCloud.tif"
OUT_TOTAL_WEIGHT_FILE="$OUT_FOLDER/WeightTotal.tif"

OUT_L3A_FILE="$OUT_FOLDER/L3AResult#.tif"

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
L3A_DATE="20130502"
HALF_SYNTHESIS="50"
WEIGHT_DATE_MIN="0.10"

MY_PWD=`pwd`

echo "Executing from $MY_PWD"

i=0
PREV_L3A=""
for xml in "${inputXML[@]}"
do

try otbcli SpotMaskHandler $COMPOSITE_OTB_LIBS_ROOT/SpotMaskHandler/ -xml $xml -out $OUT_SPOT_MASKS

try otbcli ResampleAtS2Res $COMPOSITE_OTB_LIBS_ROOT/ResampleAtS2Res/ -xml $xml -spotmask $OUT_SPOT_MASKS -outres10 $OUT_IMG_10M -outres20 $OUT_IMG_20M -outcmres10 $OUT_CLD_10M -outwmres10 $OUT_WAT_10M -outsmres10 $OUT_SNOW_10M -outaotres10 $OUT_AOT_10M -outcmres20 $OUT_CLD_20M -outwmres20 $OUT_WAT_20M -outsmres20 $OUT_SNOW_20M -outaotres20 $OUT_AOT_20M 

try otbcli WeightAOT $WEIGHT_OTB_LIBS_ROOT/WeightAOT/ -in $OUT_AOT_10M -xml $xml -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out $OUT_WEIGHT_AOT_FILE

try otbcli WeightOnClouds $WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/ -incldmsk $OUT_CLD_10M -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out $OUT_WEIGHT_CLOUD_FILE

try otbcli TotalWeight $WEIGHT_OTB_LIBS_ROOT/TotalWeight/ -xml $xml -waotfile $OUT_WEIGHT_AOT_FILE -wcldfile $OUT_WEIGHT_CLOUD_FILE -wsensor $WEIGHT_SENSOR -l3adate $L3A_DATE -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out $OUT_TOTAL_WEIGHT_FILE

mod=${OUT_L3A_FILE//[#]/$i}
i=$((i+1))
#todo... search
try otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_10M -res 10 -xml $xml $PREV_L3A -csm $OUT_CLD_10M -wm $OUT_WAT_10M -sm $OUT_SNOW_10M -wl2a $OUT_TOTAL_WEIGHT_FILE -out $mod
PREV_L3A="-prevl3a $mod"

echo "-----------------------------------------------------------"
done

rm -fr $OUT_SPOT_MASKS $OUT_IMG_10M $OUT_IMG_10M_ALL $OUT_IMG_20M $OUT_IMG_20M_ALL $OUT_CLD_10M $OUT_CLD_20M $OUT_WAT_10M $OUT_WAT_20M $OUT_SNOW_10M $OUT_SNOW_20M $OUT_AOT_10M $OUT_AOT_20M $OUT_WEIGHT_AOT_FILE $OUT_WEIGHT_CLOUD_FILE $OUT_TOTAL_WEIGHT_FILE


#otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_10M_ALL -allinone 1 -res 10 -xml $INPUT_SPOT_XML_FILE -csm $OUT_CLD_10M -wm $OUT_WAT_10M -sm $OUT_SNOW_10M -wl2a $OUT_TOTAL_WEIGHT_FILE -out $OUT_L3A_FILE_ALL

