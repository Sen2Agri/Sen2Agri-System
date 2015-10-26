#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/BelgiumS2A_20130728_L8_198_025/BelgiumS2A_20130728_L8_198_025.HDR"
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/BelgiumS2A_20130930_L8_198_025/BelgiumS2A_20130930_L8_198_025.HDR"
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/BelgiumS2A_20131203_L8_198_025/BelgiumS2A_20131203_L8_198_025.HDR"

L3A_DATE="20131203"
HALF_SYNTHESIS="75"
BANDS_MAPPING="bands_mapping_L8.txt"
#end of USER modif

if [ $# -lt 3 ]
then
  echo "Usage: $0 <otb directory application> <resolution> <out folder name> [scattering coefficient file - S2 case only]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

SCAT_COEF=""
if [ $# == 4 ] ; then    
    ./run_composite.sh "$1" "$inputXML" "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$BANDS_MAPPING" "$4"
else
    ./run_composite.sh "$1" "$inputXML" "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$BANDS_MAPPING"
fi