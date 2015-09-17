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

#USER modif
#add directories where SPOT products are to be found
inputXML[0]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000.xml'
inputXML[1]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000.xml'
inputXML[2]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml'
inputXML[3]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml'
inputXML[4]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml'
inputXML[5]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml'
inputXML[6]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000.xml'
inputXML[7]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml'
inputXML[8]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml'
inputXML[9]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml'
inputXML[10]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml'
inputXML[11]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml'
inputXML[12]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000.xml'
inputXML[13]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000.xml'
inputXML[14]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000.xml'
inputXML[15]='/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000.xml'
#end of USER modif

OUT_FOLDER=$1

VEG_STATUS_OTB_LIBS_ROOT="~/sen2agri-processors-build/VegetationStatus"
IMG_INV_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/otb-bv/src/applications"

LAI_REGRESSION_MODEL="$OUT_FOLDER/out_model.txt"
ERR_REGRESSION_MODEL="$OUT_FOLDER/out_model_errest.txt"

OUT_NDVI_RVI="$OUT_FOLDER/ndvi_rvi.tif"
OUT_LAI_IMG="$OUT_FOLDER/LAI_img_#.tif"
OUT_LAI_ERR_IMG="$OUT_FOLDER/LAI_err_img_#.tif"

OUT_LAI_TIME_SERIES="$OUT_FOLDER/LAI_time_series.tif"
OUT_ERR_TIME_SERIES="$OUT_FOLDER/Err_time_series.tif"

OUT_REPROCESSED_TIME_SERIES="$OUT_FOLDER/ReprocessedTimeSeries.tif?gdal:co:COMPRESS=DEFLATE"
OUT_FITTED_TIME_SERIES="$OUT_FOLDER/FittedTimeSeries.tif?gdal:co:COMPRESS=DEFLATE"

MY_PWD=`pwd`

echo "Executing from $MY_PWD"

cnt=0
for xml in "${inputXML[@]}"
do
    try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml -outres 10 -fts $OUT_NDVI_RVI

    CUR_OUT_LAI_IMG=${OUT_LAI_IMG//[#]/$cnt}
    CUR_OUT_LAI_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$cnt}
    
    try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -model $LAI_REGRESSION_MODEL -out $CUR_OUT_LAI_IMG
    try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -model $ERR_REGRESSION_MODEL -out $CUR_OUT_LAI_ERR_IMG
    
    rm -fr $OUT_NDVI_RVI
    
    cnt=$((cnt+1))
echo "-----------------------------------------------------------"
done

i=0

ALL_XML_PARAM=""
ALL_LAI_PARAM=""
ALL_ERR_PARAM=""
#build the parameters -ilxml for -illai
while [  $i -lt $cnt ]; do
    CUR_LAI_IMG=${OUT_LAI_IMG//[#]/$i}
    CUR_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$i}
    ALL_XML_PARAM=$ALL_XML_PARAM" "${inputXML[$i]}
    ALL_LAI_PARAM=$ALL_LAI_PARAM" "$CUR_LAI_IMG
    ALL_ERR_PARAM=$ALL_ERR_PARAM" "$CUR_ERR_IMG
    
    i=$((i+1))
done

# Create the LAI and Error time series
try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_LAI_PARAM -out $OUT_LAI_TIME_SERIES
try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_ERR_PARAM -out $OUT_ERR_TIME_SERIES

rm -fr $ALL_LAI_PARAM $ALL_ERR_PARAM

# Compute the reprocessed time series (On-line Retrieval)
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_REPROCESSED_TIME_SERIES  -algo local -algo.local.bwr 2 -algo.local.fwr 0

# Compute the fitted time series (CSDM Fitting)
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_FITTED_TIME_SERIES -algo fit

rm -fr $OUT_LAI_TIME_SERIES $OUT_ERR_TIME_SERIES