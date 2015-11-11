#! /bin/bash

USE_COMPRESSION=1

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
#source try_command.sh

#USER modif
#add directories where SPOT products are to be found
#source $1
#end of USER modif

IFS=' ' read -a inputXML <<< "$2"

RESOLUTION=$3
OUT_FOLDER=$4
mkdir "$OUT_FOLDER"
L3A_DATE=$5
HALF_SYNTHESIS=$6


if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]]
then
  echo "The resolution is : "$RESOLUTION
  echo "The resolution should be either 10 or 20."
  echo "The product will be created with the original resolution without resampling."
fi

COMPOSITE_OTB_LIBS_ROOT="$1"
PRODUCT_FORMATER_OTB_LIBS_ROOT="$1/../MACCSMetadata/src"

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

FULL_SCAT_COEFFS=""
BANDS_MAPPING="$7"

cp "$BANDS_MAPPING" "$OUT_FOLDER"

FULL_BANDS_MAPPING="$OUT_FOLDER/$BANDS_MAPPING"

if [ $# == 8 ] ; then
    SCAT_COEFFS="$7"
#"scattering_coeffs_"$RESOLUTION"m.txt"
    cp "$SCAT_COEFFS" "$OUT_FOLDER"
    FULL_SCAT_COEFFS="-scatcoef $OUT_FOLDER/$SCAT_COEFFS"
fi


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
LAST_XML=""
for xml in "${inputXML[@]}"
do
    echo "$xml" >> $PARAMS_TXT

    try otbcli MaskHandler "$COMPOSITE_OTB_LIBS_ROOT/MaskHandler/" -xml "$xml" -out "$OUT_SPOT_MASKS" -sentinelres $RESOLUTION
    
    mod=${OUT_L3A_FILE//[#]/$i}
    out_w=${OUT_WEIGHTS//[#]/$i}
    out_d=${OUT_DATES//[#]/$i}
    out_r=${OUT_REFLS//[#]/$i}
    out_f=${OUT_FLAGS//[#]/$i}
    out_rgb=${OUT_RGB//[#]/$i}
    i=$((i+1))

    try otbcli CompositePreprocessing2 "$COMPOSITE_OTB_LIBS_ROOT/CompositePreprocessing/" -xml "$xml" -bmap "$FULL_BANDS_MAPPING" -res "$RESOLUTION" "$FULL_SCAT_COEFFS" -msk "$OUT_SPOT_MASKS" -outres "$OUT_IMG_BANDS" -outcmres "$OUT_CLD" -outwmres "$OUT_WAT" -outsmres "$OUT_SNOW" -outaotres "$OUT_AOT"

    try otbcli WeightAOT "$WEIGHT_OTB_LIBS_ROOT/WeightAOT/" -in "$OUT_AOT" -xml "$xml" -waotmin "$WEIGHT_AOT_MIN" -waotmax "$WEIGHT_AOT_MAX" -aotmax "$AOT_MAX" -out "$OUT_WEIGHT_AOT_FILE"

    try otbcli WeightOnClouds "$WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/" -incldmsk "$OUT_CLD" -coarseres "$COARSE_RES" -sigmasmallcld "$SIGMA_SMALL_CLD" -sigmalargecld "$SIGMA_LARGE_CLD" -out "$OUT_WEIGHT_CLOUD_FILE"

    try otbcli TotalWeight "$WEIGHT_OTB_LIBS_ROOT/TotalWeight/" -xml "$xml" -waotfile "$OUT_WEIGHT_AOT_FILE" -wcldfile "$OUT_WEIGHT_CLOUD_FILE" -wsensor "$WEIGHT_SENSOR" -l3adate "$L3A_DATE" -halfsynthesis "$HALF_SYNTHESIS" -wdatemin "$WEIGHT_DATE_MIN" -out "$OUT_TOTAL_WEIGHT_FILE"

    #todo... search for previous L3A product?
    try otbcli UpdateSynthesis2 "$COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/" -in "$OUT_IMG_BANDS" -bmap "$FULL_BANDS_MAPPING" -xml "$xml" $PREV_L3A -csm "$OUT_CLD" -wm "$OUT_WAT" -sm "$OUT_SNOW" -wl2a "$OUT_TOTAL_WEIGHT_FILE" -out "$mod"

    if [ USE_COMPRESSION == 0 ] ; then
        try otbcli CompositeSplitter2 "$COMPOSITE_OTB_LIBS_ROOT/CompositeSplitter/" -in "$mod" -xml "$xml" -bmap "$FULL_BANDS_MAPPING" -outweights "$out_w" -outdates "$out_d" -outrefls "$out_r" -outflags "$out_f" -outrgb "$out_rgb"    
    else
        try otbcli CompositeSplitter2 "$COMPOSITE_OTB_LIBS_ROOT/CompositeSplitter/" -in "$mod" -xml "$xml" -bmap "$FULL_BANDS_MAPPING" -outweights "$out_w?gdal:co:COMPRESS=DEFLATE" -outdates "$out_d?gdal:co:COMPRESS=DEFLATE" -outrefls "$out_r?gdal:co:COMPRESS=DEFLATE" -outflags "$out_f?gdal:co:COMPRESS=DEFLATE" -outrgb "$out_rgb?gdal:co:COMPRESS=DEFLATE"
    fi
	LAST_XML=$xml

    #PREV_L3A="-prevl3a $mod"
    PREV_L3A="-prevl3aw $out_w -prevl3ad $out_d -prevl3ar $out_r -prevl3af $out_f"
    
echo "-----------------------------------------------------------"
done

try otbcli ProductFormatter "$PRODUCT_FORMATER_OTB_LIBS_ROOT"  -destroot "$OUT_FOLDER" -fileclass SVT1 -level L3A -timeperiod $L3A_DATE -baseline 01.00 -processor composite -processor.composite.refls "$out_r" -processor.composite.weights "$out_w" -processor.composite.flags "$out_f" -processor.composite.dates "$out_d" -processor.composite.rgb "$out_rgb" -il "$xml" -gipp "$PARAMS_TXT"

rm -fr $OUT_SPOT_MASKS $OUT_IMG_BANDS $OUT_IMG_BANDS_ALL $OUT_CLD $OUT_WAT $OUT_SNOW $OUT_AOT $OUT_WEIGHT_AOT_FILE $OUT_WEIGHT_CLOUD_FILE $OUT_TOTAL_WEIGHT_FILE


