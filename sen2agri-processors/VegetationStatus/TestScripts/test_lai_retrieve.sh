#! /bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: $0 <include file> <out folder name> [models folder name]"
  echo "The file with input xmls should be given. The output directory should also be given" 1>&2  
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
#add directories where products are to be found
source $1
#end of USER modif

OUT_FOLDER=$2
MODELS_FOLDER=${OUT_FOLDER}
if [ $# -gt 2 ] ; then
    MODELS_FOLDER=$3
fi

VEG_STATUS_OTB_LIBS_ROOT="~/sen2agri-processors-build/VegetationStatus"
IMG_INV_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/otb-bv/src/applications"

OUT_NDVI_RVI="$OUT_FOLDER/ndvi_rvi.tif"
OUT_LAI_IMG="$OUT_FOLDER/LAI_img_#.tif"
OUT_LAI_ERR_IMG="$OUT_FOLDER/LAI_err_img_#.tif"

OUT_LAI_TIME_SERIES="$OUT_FOLDER/LAI_time_series.tif"
OUT_ERR_TIME_SERIES="$OUT_FOLDER/Err_time_series.tif"

OUT_REPROCESSED_TIME_SERIES="$OUT_FOLDER/ReprocessedTimeSeries.tif"
OUT_FITTED_TIME_SERIES="$OUT_FOLDER/FittedTimeSeries.tif"

MY_PWD=`pwd`

echo "Executing from $MY_PWD"
echo "Models folder: $MODELS_FOLDER"

MODELS_INPUT_LIST=""
for entry in $MODELS_FOLDER/Model_*.txt
do
    echo "Model file: $entry"
    MODELS_INPUT_LIST="$MODELS_INPUT_LIST $entry"
done

ERR_MODELS_INPUT_LIST=""
for entry in $MODELS_FOLDER/Err_Est_Model_*.txt
do
    echo "Err estimation file: $entry"
    ERR_MODELS_INPUT_LIST="$ERR_MODELS_INPUT_LIST $entry"
done

MODEL_FILE="$OUT_FOLDER/model_file.txt"
ERR_MODEL_FILE="$OUT_FOLDER/err_model_file.txt"

cnt=0
for xml in "${inputXML[@]}"
do
    #try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml -outres 10 -fts $OUT_NDVI_RVI
    try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml -fts $OUT_NDVI_RVI

    try otbcli GetLaiRetrievalModel $IMG_INV_OTB_LIBS_ROOT -xml $xml -ilmodels $MODELS_INPUT_LIST -ilerrmodels $ERR_MODELS_INPUT_LIST -outm $MODEL_FILE -outerr $ERR_MODEL_FILE
    
    CUR_OUT_LAI_IMG=${OUT_LAI_IMG//[#]/$cnt}
    CUR_OUT_LAI_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$cnt}
    
    try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $MODEL_FILE -out $CUR_OUT_LAI_IMG
    try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $ERR_MODEL_FILE -out $CUR_OUT_LAI_ERR_IMG
    
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

# Compute the reprocessed time series (On-line Retrieval)
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_REPROCESSED_TIME_SERIES  -algo local -algo.local.bwr 2 -algo.local.fwr 0

# Compute the fitted time series (CSDM Fitting)
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_FITTED_TIME_SERIES -algo fit
