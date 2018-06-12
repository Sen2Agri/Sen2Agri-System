#! /bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: $0 <resolution> <out folder name>"
  echo "The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
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

#USER modif
#add directories where SPOT products are to be found
inputXML[0]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000.xml'
inputXML[1]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000.xml'
inputXML[2]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml'
inputXML[3]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml'
inputXML[4]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml'
inputXML[5]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml'
inputXML[6]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000.xml'
inputXML[7]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml'
inputXML[8]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml'
inputXML[9]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml'
inputXML[10]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml'
inputXML[11]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml'
inputXML[12]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000.xml'
inputXML[13]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000.xml'
inputXML[14]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000.xml'
inputXML[15]='/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000.xml'

#end of USER modif

RESOLUTION=$1
OUT_FOLDER=$2

if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]]
then
  echo $RESOLUTION
  echo "Check the resolution, should be either 10 or 20"
  exit 1
fi

COMPOSITE_OTB_LIBS_ROOT="~/sen2agri-processors-build/Composite"

WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"

OUT_SPOT_MASKS="$OUT_FOLDER/spot_masks.tif"

OUT_IMG_BANDS="$OUT_FOLDER/res$RESOLUTION.tif"
OUT_IMG_BANDS_ALL="$OUT_FOLDER/res"$RESOLUTION"_all.tif"
OUT_CLD="$OUT_FOLDER/cld$RESOLUTION.tif"
OUT_WAT="$OUT_FOLDER/wat$RESOLUTION.tif"
OUT_SNOW="$OUT_FOLDER/snow$RESOLUTION.tif"
OUT_AOT="$OUT_FOLDER/aot$RESOLUTION.tif"

OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tif"
OUT_WEIGHT_CLOUD_FILE="$OUT_FOLDER/WeightCloud.tif"
OUT_TOTAL_WEIGHT_FILE="$OUT_FOLDER/WeightTotal.tif"

OUT_L3A_FILE="$OUT_FOLDER/L3AResult_Sumipy#_"$RESOLUTION"M.tif"

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
L3A_DATE="20130616"
HALF_SYNTHESIS="50"
WEIGHT_DATE_MIN="0.10"

MY_PWD=`pwd`

echo "Executing from $MY_PWD"

i=0
PREV_L3A=""
for xml in "${inputXML[@]}"
do

try otbcli SpotMaskHandler $COMPOSITE_OTB_LIBS_ROOT/SpotMaskHandler/ -xml $xml -out $OUT_SPOT_MASKS

try otbcli ResampleAtS2Res $COMPOSITE_OTB_LIBS_ROOT/ResampleAtS2Res/ -xml $xml -spotmask $OUT_SPOT_MASKS -outres$RESOLUTION $OUT_IMG_BANDS -outcmres$RESOLUTION $OUT_CLD -outwmres$RESOLUTION $OUT_WAT -outsmres$RESOLUTION $OUT_SNOW -outaotres$RESOLUTION $OUT_AOT

try otbcli WeightAOT $WEIGHT_OTB_LIBS_ROOT/WeightAOT/ -in $OUT_AOT -xml $xml -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out $OUT_WEIGHT_AOT_FILE

try otbcli WeightOnClouds $WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/ -incldmsk $OUT_CLD -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out $OUT_WEIGHT_CLOUD_FILE

try otbcli TotalWeight $WEIGHT_OTB_LIBS_ROOT/TotalWeight/ -xml $xml -waotfile $OUT_WEIGHT_AOT_FILE -wcldfile $OUT_WEIGHT_CLOUD_FILE -wsensor $WEIGHT_SENSOR -l3adate $L3A_DATE -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out $OUT_TOTAL_WEIGHT_FILE

mod=${OUT_L3A_FILE//[#]/$i}
i=$((i+1))
#todo... search for previous L3A product?
try otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_BANDS -res $RESOLUTION -xml $xml $PREV_L3A -csm $OUT_CLD -wm $OUT_WAT -sm $OUT_SNOW -wl2a $OUT_TOTAL_WEIGHT_FILE -out $mod
PREV_L3A="-prevl3a $mod"

echo "-----------------------------------------------------------"
done

#rm -fr $OUT_SPOT_MASKS $OUT_IMG_BANDS $OUT_IMG_BANDS_ALL $OUT_CLD $OUT_WAT $OUT_SNOW $OUT_AOT $OUT_WEIGHT_AOT_FILE $OUT_WEIGHT_CLOUD_FILE $OUT_TOTAL_WEIGHT_FILE


#otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_10M_ALL -allinone 1 -res 10 -xml $INPUT_SPOT_XML_FILE -csm $OUT_CLD_10M -wm $OUT_WAT_10M -sm $OUT_SNOW_10M -wl2a $OUT_TOTAL_WEIGHT_FILE -out $OUT_L3A_FILE_ALL

