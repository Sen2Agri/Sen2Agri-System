#! /bin/bash

if [ $# -lt 1 ]
then
  echo "Usage: $0 <folder name>"
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
inputXML[0]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml'
inputXML[1]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000.xml'
inputXML[2]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000.xml'
inputXML[3]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000.xml'
inputXML[4]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000.xml'
inputXML[5]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000.xml'
inputXML[6]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000.xml'
inputXML[7]='/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000.xml'

#end of USER modif

#RESOLUTION=$1
OUT_FOLDER=$1

CROPTTYPE_OTB_LIBS_ROOT="~/sen2agri-processors-build/CropType"
VEGETATIONSTATUS_OTB_LIBS_ROOT="~/sen2agri-processors-build/VegetationStatus/phenotb/src/Applications/"
OUT_BANDS="$OUT_FOLDER/spot_bands.TIF"
OUT_MASKS="$OUT_FOLDER/spot_masks.TIF"
OUT_DATES="$OUT_FOLDER/spot_dates.txt"
OUT_SHAPE="$OUT_FOLDER/spot_shapes.shp"
OUT_NDVI="$OUT_FOLDER/spot_ndvi.TIF"
OUT_SIGMO="$OUT_FOLDER/spot_sigmo.TIF"
OUT_METRIC="$OUT_FOLDER/metric_estimation.TIF"

OUT_L3A_FILE="$OUT_FOLDER/L3AResult.tif"

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
try otbcli BandsExtractor $CROPTTYPE_OTB_LIBS_ROOT/BandsExtractor/ -il $TIME_SERIES_XMLS -out $OUT_BANDS -allmasks $OUT_MASKS -outdate $OUT_DATES
try otbcli FeatureExtraction $CROPTTYPE_OTB_LIBS_ROOT/FeatureExtraction -rtocr $OUT_BANDS -ndvi $OUT_NDVI
try otbcli SigmoFitting $VEGETATIONSTATUS_OTB_LIBS_ROOT -in $OUT_NDVI -mask $OUT_MASKS -dates $OUT_DATES -out $OUT_SIGMO
try otbcli MetricsEstimation $VEGETATIONSTATUS_OTB_LIBS_ROOT -ipf $OUT_SIGMO -indates $OUT_DATES -opf $OUT_METRIC

#rm -fr $OUT_MASKS $OUT_BANDS $OUT_DATES $OUT_NDVI $OUT_SIGMO
