#! /bin/bash

if [ $# -lt 3 ]
then
  echo "Usage: $0 <croptype otb application directory> <pheno metrics otb application directory> <input xmls list> <output directory name>"
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
	  #  if [[ ! $(diff "$1" "$COMPARISION_FILE") ]] ; then
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


#USER modif
#add directories where SPOT products are to be found
#source $1
#end of USER modif

IFS=' ' read -a inputXML <<< "$3"

OUT_FOLDER=$4
mkdir "$OUT_FOLDER"

CROPTTYPE_OTB_LIBS_ROOT=$1
#"~/sen2agri-processors-build/CropType"
VEGETATIONSTATUS_OTB_LIBS_ROOT=$2
#"~/sen2agri-processors-build/VegetationStatus/phenotb/src/Applications/"
OUT_BANDS="$OUT_FOLDER/spot_bands.tif"
OUT_MASKS="$OUT_FOLDER/spot_masks.tif"
OUT_DATES="$OUT_FOLDER/spot_dates.txt"
OUT_SHAPE="$OUT_FOLDER/spot_shapes.shp"
OUT_NDVI="$OUT_FOLDER/spot_ndvi.tif"
OUT_SIGMO="$OUT_FOLDER/spot_sigmo.tif"
OUT_METRIC="$OUT_FOLDER/metric_estimation.tif"

MY_PWD=`pwd`

echo "Executing from $MY_PWD"

i=0
TIME_SERIES_XMLS=""
for xml in "${inputXML[@]}"
do

TIME_SERIES_XMLS+=$xml
TIME_SERIES_XMLS+=" "

done
echo "time_series = $TIME_SERIES_XMLS"
try otbcli BandsExtractor $CROPTTYPE_OTB_LIBS_ROOT/BandsExtractor/ -il $TIME_SERIES_XMLS -merge true -ndh true -out $OUT_BANDS -allmasks $OUT_MASKS -outdate $OUT_DATES
try otbcli FeatureExtraction $CROPTTYPE_OTB_LIBS_ROOT/FeatureExtraction -rtocr $OUT_BANDS -ndvi $OUT_NDVI
try otbcli SigmoFitting $VEGETATIONSTATUS_OTB_LIBS_ROOT -in $OUT_NDVI -mask $OUT_MASKS -dates $OUT_DATES -out $OUT_SIGMO
if [[ $# == 4 && "$4" == "tc3-1" ]] ; then
ut_output_info "$OUT_SIGMO" 2 "./qr_cmp_pheno_southafrica/spot_sigmo.tif" 8008372
rm -fr $OUT_MASKS $OUT_BANDS $OUT_DATES $OUT_NDVI 
exit
fi
try otbcli MetricsEstimation $VEGETATIONSTATUS_OTB_LIBS_ROOT -ipf $OUT_SIGMO -indates $OUT_DATES -opf $OUT_METRIC
if [[ $# == 4 && "$4" == "tc3-2" ]] ; then
ut_output_info "$OUT_SIGMO" 2 "./qr_cmp_pheno_southafrica/metric_estimation.tif" 8008372
fi

rm -fr $OUT_MASKS $OUT_BANDS $OUT_DATES $OUT_NDVI 
#$OUT_SIGMO
