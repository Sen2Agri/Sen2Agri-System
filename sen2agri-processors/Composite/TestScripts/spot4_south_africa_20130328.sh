#!/bin/bash
#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml "


#L3A_DATE="20130617"
L3A_DATE="20130328"
#HALF_SYNTHESIS="50"
HALF_SYNTHESIS="15"
BANDS_MAPPING="bands_mapping_spot.txt"
#end of USER modif

if [ $# -lt 4 ]
then
  echo "Usage: $0 <otb directory application> <resolution> <out folder name> <bands mapping file> [scattering coefficient file - S2 case only]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

SCAT_COEF=""
if [ $# == 5 ] ; then    
    ./run_composite.sh "$1" "$inputXML " "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$4" "$5"
else
    ./run_composite.sh "$1" "$inputXML " "$2" "/mnt/scratch/composite_southafrica_20130328" "$L3A_DATE" "$HALF_SYNTHESIS" "$4"
fi

