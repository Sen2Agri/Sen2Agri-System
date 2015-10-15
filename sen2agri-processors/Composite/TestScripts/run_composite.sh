#! /bin/bash

if [ $# -lt 3 ]
then
  echo "Usage: $0 <include file> <resolution> <out folder name> [combined] [allinone]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
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
source $1
#end of USER modif

RESOLUTION=$2
OUT_FOLDER=$3
ALL_IN_ONE=""
IS_SINGLE_PRODUCTS_TYPE_MODE="-stypemode 1"

if [ $# -gt 3 ] ; then
    IS_SINGLE_PRODUCTS_TYPE_MODE=""
fi


if [ $# -gt 4 ] ; then
    ALL_IN_ONE="-allinone 1"
fi

if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]]
then
  echo "The resolution is : "$RESOLUTION
  echo "The resolution should be either 10 or 20."
  echo "The product will be created with the original resolution without resampling."
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

OUT_L3A_FILE="$OUT_FOLDER/L3AResult#_"$RESOLUTION"M.tif"

PARAMS_TXT="$OUT_FOLDER/params.txt"
rm -fr $PARAMS_TXT
touch $PARAMS_TXT

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
WEIGHT_DATE_MIN="0.10"

OUT_WEIGHTS="$OUT_FOLDER/L3AResult#_weights.tif"
OUT_DATES="$OUT_FOLDER/L3AResult#_dates.tif"
OUT_REFLS="$OUT_FOLDER/L3AResult#_refls.tif"
OUT_FLAGS="$OUT_FOLDER/L3AResult#_flags.tif"
OUT_RGB="$OUT_FOLDER/L3AResult#_rgb.tif"

SCAT_COEFFS="scattering_coefs_$RESOLUTION.txt"

MY_PWD=`pwd`

echo "Weight AOT" >> $PARAMS_TXT
echo "    weight aot min    = $WEIGHT_AOT_MIN" >> $PARAMS_TXT
echo "    weight aot max    = $WEIGHT_AOT_MAX" >> $PARAMS_TXT
echo "    aot max           = $AOT_MAX" >> $PARAMS_TXT
echo "Weight on clouds" >> $PARAMS_TXT
echo "    coarse res        = $COARSE_RES" >> $PARAMS_TXT
echo "    sigma small cloud = $SIGMA_SMALL_CLD" >> $PARAMS_TXT
echo "    sigma large cloud = $SIGMA_LARGE_CLD" >> $PARAMS_TXT
echo "Weight on Date" >> $PARAMS_TXT
echo "    weight date min   = $WEIGHT_DATE_MIN" >> $PARAMS_TXT
echo "    l3a product date  = $L3A_DATE" >> $PARAMS_TXT
echo "    half synthesis    = $HALF_SYNTHESIS" >> $PARAMS_TXT
echo "Weight on " >> $PARAMS_TXT
echo "    weight sensor     = $WEIGHT_SENSOR" >> $PARAMS_TXT
echo " " >> $PARAMS_TXT
echo "Used XML files " >> $PARAMS_TXT

echo "Executing from $MY_PWD"

i=0
PREV_L3A=""
for xml in "${inputXML[@]}"
do
    echo "$xml" >> $PARAMS_TXT
    
    try otbcli MaskHandler $COMPOSITE_OTB_LIBS_ROOT/MaskHandler/ -xml $xml -out $OUT_SPOT_MASKS
    
    mod=${OUT_L3A_FILE//[#]/$i}
    out_w=${OUT_WEIGHTS//[#]/$i}
    out_d=${OUT_DATES//[#]/$i}
    out_r=${OUT_REFLS//[#]/$i}
    out_f=${OUT_FLAGS//[#]/$i}
    out_rgb="-outrgb ${OUT_RGB//[#]/$i}"
    i=$((i+1))
    
    if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]] ; then
#        try otbcli ResampleAtS2Res $COMPOSITE_OTB_LIBS_ROOT/ResampleAtS2Res/ -xml $xml -spotmask $OUT_SPOT_MASKS -outres $OUT_IMG_BANDS -outcmres $OUT_CLD -outwmres $OUT_WAT -outsmres $OUT_SNOW -outaotres $OUT_AOT

	try otbcli CompositePreprocessing $COMPOSITE_OTB_LIBS_ROOT/CompositePreprocessing/ -xml $xml -res $RESOLUTION -scatcoef $SCAT_COEFFS -msk $OUT_SPOT_MASKS -outres $OUT_IMG_BANDS -outcmres $OUT_CLD -outwmres $OUT_WAT -outsmres $OUT_SNOW -outaotres $OUT_AOT

        try otbcli WeightAOT $WEIGHT_OTB_LIBS_ROOT/WeightAOT/ -in $OUT_AOT -xml $xml -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out $OUT_WEIGHT_AOT_FILE

        try otbcli WeightOnClouds $WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/ -incldmsk $OUT_CLD -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out $OUT_WEIGHT_CLOUD_FILE

        try otbcli TotalWeight $WEIGHT_OTB_LIBS_ROOT/TotalWeight/ -xml $xml -waotfile $OUT_WEIGHT_AOT_FILE -wcldfile $OUT_WEIGHT_CLOUD_FILE -wsensor $WEIGHT_SENSOR -l3adate $L3A_DATE -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out $OUT_TOTAL_WEIGHT_FILE

        #todo... search for previous L3A product?
        try otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_BANDS -allinone 1 -xml $xml $PREV_L3A -csm $OUT_CLD -wm $OUT_WAT -sm $OUT_SNOW -wl2a $OUT_TOTAL_WEIGHT_FILE -out $mod $IS_SINGLE_PRODUCTS_TYPE_MODE

        try otbcli CompositeSplitter $COMPOSITE_OTB_LIBS_ROOT/CompositeSplitter/ -in $mod -allinone 1 -xml $xml -outweights $out_w -outdates $out_d -outrefls $out_r -outflags $out_f $IS_SINGLE_PRODUCTS_TYPE_MODE $out_rgb
    else 
        #try otbcli ResampleAtS2Res $COMPOSITE_OTB_LIBS_ROOT/ResampleAtS2Res/ -xml $xml -spotmask $OUT_SPOT_MASKS -outres$RESOLUTION $OUT_IMG_BANDS -outcmres$RESOLUTION $OUT_CLD -outwmres$RESOLUTION $OUT_WAT -outsmres$RESOLUTION $OUT_SNOW -outaotres$RESOLUTION $OUT_AOT $ALL_IN_ONE
        if [[ $RESOLUTION == 20 ]] ; then
	    out_rgb=""
	fi   

	try otbcli CompositePreprocessing $COMPOSITE_OTB_LIBS_ROOT/CompositePreprocessing/ -xml $xml -res $RESOLUTION -scatcoef $SCAT_COEFFS -msk $OUT_SPOT_MASKS -outres $OUT_IMG_BANDS -outcmres $OUT_CLD -outwmres $OUT_WAT -outsmres $OUT_SNOW -outaotres $OUT_AOT

        try otbcli WeightAOT $WEIGHT_OTB_LIBS_ROOT/WeightAOT/ -in $OUT_AOT -xml $xml -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out $OUT_WEIGHT_AOT_FILE

        try otbcli WeightOnClouds $WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/ -incldmsk $OUT_CLD -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out $OUT_WEIGHT_CLOUD_FILE

        try otbcli TotalWeight $WEIGHT_OTB_LIBS_ROOT/TotalWeight/ -xml $xml -waotfile $OUT_WEIGHT_AOT_FILE -wcldfile $OUT_WEIGHT_CLOUD_FILE -wsensor $WEIGHT_SENSOR -l3adate $L3A_DATE -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out $OUT_TOTAL_WEIGHT_FILE

        #todo... search for previous L3A product?
        try otbcli UpdateSynthesis $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in $OUT_IMG_BANDS -xml $xml $PREV_L3A -csm $OUT_CLD -wm $OUT_WAT -sm $OUT_SNOW -wl2a $OUT_TOTAL_WEIGHT_FILE -out $mod $IS_SINGLE_PRODUCTS_TYPE_MODE $ALL_IN_ONE

        try otbcli CompositeSplitter $COMPOSITE_OTB_LIBS_ROOT/CompositeSplitter/ -in $mod -res $RESOLUTION -xml $xml -outweights $out_w -outdates $out_d -outrefls $out_r -outflags $out_f $IS_SINGLE_PRODUCTS_TYPE_MODE $ALL_IN_ONE $out_rgb
    fi 
    #PREV_L3A="-prevl3a $mod"
    PREV_L3A="-prevl3aw $out_w -prevl3ad $out_d -prevl3ar $out_r -prevl3af $out_f"
    
echo "-----------------------------------------------------------"
done

rm -fr $OUT_SPOT_MASKS $OUT_IMG_BANDS $OUT_IMG_BANDS_ALL $OUT_CLD $OUT_WAT $OUT_SNOW $OUT_AOT $OUT_WEIGHT_AOT_FILE $OUT_WEIGHT_CLOUD_FILE $OUT_TOTAL_WEIGHT_FILE


