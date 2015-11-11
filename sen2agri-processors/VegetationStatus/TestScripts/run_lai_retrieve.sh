#! /bin/bash

if [ $# -lt 9 ]
then
  echo "Usage: $0 <otb application directory> <list of input XMLs> <resolution> <out folder name> <RSR filename> <solar zenith angle> <sensor_zenith_angle> <relative azimuth angle> <scripts folder name>"
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

function ut_output_info {
    if [ $# != 4 ] ; then
	echo " Wrong call for ut_output_info. This function should receive 4 params"
	exit
    fi
    echo "Information for $1 file:"
    OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $1 | grep "Number of bands")"

    BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"
    COMPARISION_FILE="$3"
#"./qr_cmp_southafrica/ReprocessedTimeSeries.tif"

    if [ $BANDS_NB == $2 ] ; then
	echo "Number of bands: PASSED"
	FILESIZE=$(stat -c%s "$1")
	if [ $FILESIZE == $4 ] ; then    
	    echo "File size      : PASSED"
	    #if [[ ! $(diff "$1" "$COMPARISION_FILE") ]] ; then
#otbcli_CompareImages -ref.in /mnt/data/output_temp/out_qr_vegetation_south_africa//FittedTimeSeries.tif -meas.in qr_cmp_southafrica/FittedTimeSeries.tif | grep psnr | cut "-d " -f2
	#	echo "Comp ref file  : PASSED"
	 #   else
	#	echo "Comp ref file  : FAILED"
	 #   fi
	else
	    echo "File size      : FAILED"
	fi
    else
	echo "Number of bands: FAILED"
	echo "File size      : FAILED"
	#echo "Comp ref file  : FAILED"
    fi

}

function timed_exec {
    IN_PARAM="$1"
    echo "EXECUTING: $IN_PARAM" >> "$OUT_FOLDER/Execution_Times.txt"
    start=`date +%s`
    ($IN_PARAM)
    end=`date +%s`

    execution_time=$((end-start))
    echo -e "\nExecution took: $execution_time seconds\n\n" >> "$OUT_FOLDER/Execution_Times.txt"
}

IFS=' ' read -a inputXML <<< "$2"
RESOLUTION=$3

OUT_FOLDER=$4
mkdir "$OUT_FOLDER"

RSR_FILENAME="$5"
SOLAR_ZENITH_ANGLE=$6
SENSOR_ZENITH_ANGLE=$7
RELATIVE_AZIMUTH_ANGLE=$8
MODELS_FOLDER=${OUT_FOLDER}

VEG_STATUS_OTB_LIBS_ROOT="$1"
PRODUCT_FORMATER_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/../MACCSMetadata/src"

"$9/lai_model.sh" "$VEG_STATUS_OTB_LIBS_ROOT" "$RSR_FILENAME" $SOLAR_ZENITH_ANGLE $SENSOR_ZENITH_ANGLE $RELATIVE_AZIMUTH_ANGLE "$MODELS_FOLDER"
exit 0
echo "Starting tests ..." > "$OUT_FOLDER/Execution_Times.txt"

RESOLUTION_OPTION="-outres $RESOLUTION"
if [[ $RESOLUTION != 10 && $RESOLUTION != 20 ]]
then
  echo "The resolution is : "$RESOLUTION
  echo "The resolution should be either 10 or 20."
  echo "The product will be created with the original resolution without resampling."
  RESOLUTION_OPTION=""
fi



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
    
    timed_exec "try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml $RESOLUTION_OPTION -fts $OUT_NDVI_RVI"

    timed_exec "try otbcli GetLaiRetrievalModel $IMG_INV_OTB_LIBS_ROOT -xml $xml -ilmodels $MODELS_INPUT_LIST -ilerrmodels $ERR_MODELS_INPUT_LIST -outm $MODEL_FILE -outerr $ERR_MODEL_FILE"
    
    CUR_OUT_LAI_IMG=${OUT_LAI_IMG//[#]/$cnt}
    CUR_OUT_LAI_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$cnt}
    
    timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $MODEL_FILE -out $CUR_OUT_LAI_IMG"
    timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $ERR_MODEL_FILE -out $CUR_OUT_LAI_ERR_IMG"
    
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
timed_exec "try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_LAI_PARAM -out $OUT_LAI_TIME_SERIES"
timed_exec "try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_ERR_PARAM -out $OUT_ERR_TIME_SERIES"

if [[ $# == 10 && "$10" == "tc2-1" ]] ; then
ut_output_info "$OUT_LAI_TIME_SERIES" 5 "./qr_cmp_southafrica/LAI_time_series.tif" 20008400
ut_output_info "$OUT_ERR_TIME_SERIES" 5 "./qr_cmp_southafrica/Err_time_series.tif" 20008400
exit
fi

# Compute the reprocessed time series (On-line Retrieval)
timed_exec "try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_REPROCESSED_TIME_SERIES  -algo local -algo.local.bwr $ALGO_LOCAL_BWR -algo.local.fwr $ALGO_LOCAL_FWR"

if [[ $# == 10 && "$10" == "tc2-2" ]] ; then
ut_output_info "$OUT_REPROCESSED_TIME_SERIES" 2 "./qr_cmp_southafrica/ReprocessedTimeSeries.tif" 8008372
exit
fi

#split the Reprocessed time series to a number of images
timed_exec "try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_REPROCESSED_TIME_SERIES -outlist $REPROCESSED_LIST_FILE -compress 1"


# Compute the fitted time series (CSDM Fitting)
timed_exec "try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_FITTED_TIME_SERIES -algo fit"

if [[ $# == 10 && "$10" == "tc2-3" ]] ; then
ut_output_info "$OUT_FITTED_TIME_SERIES" 2 "./qr_cmp_southafrica/FittedTimeSeries.tif" 8008372
exit
fi

try otbcli ProductFormatter "$PRODUCT_FORMATER_OTB_LIBS_ROOT" -destroot "$OUT_FOLDER" -fileclass SVT1 -level L3B -timeperiod 20130228_20130615 -baseline 01.00 -processor vegetation -processor.vegetation.lairepr "$OUT_REPROCESSED_TIME_SERIES" -processor.vegetation.laifit "$OUT_FITTED_TIME_SERIES" -il "${inputXML[0]}" -gipp "$PARAMS_TXT"

#split the Fitted time series to a number of images
timed_exec "try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_FITTED_TIME_SERIES -outlist $FITTED_LIST_FILE -compress 1"
