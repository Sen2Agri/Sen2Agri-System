#! /bin/bash

if [ $# -lt 3 ]
then
  echo "Usage: $0 <include file> <resolution> <out folder name> [models folder name]"
  echo "The file with input xmls should be given. The resolution should be given. The output directory should also be given" 1>&2  
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

IFS=' ' read -a inputXML <<< "$1"
RESOLUTION=$2

OUT_FOLDER=$3
MODELS_FOLDER=${OUT_FOLDER}
if [ $# -gt 3 ] ; then
    MODELS_FOLDER=$4
fi

RESOLUTION_OPTION="-outres $RESOLUTION"
if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]]
then
  echo "The resolution is : "$RESOLUTION
  echo "The resolution should be either 10 or 20."
  echo "The product will be created with the original resolution without resampling."
  RESOLUTION_OPTION=""
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

FITTED_LIST_FILE="$OUT_FOLDER/FittedFilesList.txt"
REPROCESSED_LIST_FILE="$OUT_FOLDER/ReprocessedFilesList.txt"

#ProfileReprocessing parameters
ALGO_LOCAL_BWR=2
ALGO_LOCAL_FWR=0

PARAMS_TXT="$OUT_FOLDER/lai_retrieval_params.txt"
rm -fr $PARAMS_TXT
touch $PARAMS_TXT

echo "ProfileReprocessing parameters" >> $PARAMS_TXT
echo "    bwr for algo local (online retrieval) = $ALGO_LOCAL_BWR" >> $PARAMS_TXT
echo "    fwr for algo local (online retrieval) = $ALGO_LOCAL_FWR" >> $PARAMS_TXT
echo " " >> $PARAMS_TXT
echo "Used XML files " >> $PARAMS_TXT


cnt=0
for xml in "${inputXML[@]}"
do
    echo "$xml" >> $PARAMS_TXT
    
    try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml $RESOLUTION_OPTION -fts $OUT_NDVI_RVI

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
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_REPROCESSED_TIME_SERIES  -algo local -algo.local.bwr $ALGO_LOCAL_BWR -algo.local.fwr $ALGO_LOCAL_FWR

#split the Reprocessed time series to a number of images
try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_REPROCESSED_TIME_SERIES -outlist $REPROCESSED_LIST_FILE

# Compute the fitted time series (CSDM Fitting)
try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_FITTED_TIME_SERIES -algo fit

#split the Fitted time series to a number of images
try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_FITTED_TIME_SERIES -outlist $FITTED_LIST_FILE
