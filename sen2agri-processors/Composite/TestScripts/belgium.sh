#! /bin/bash
#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000.xml "
inputXML+="/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000.xml "


L3A_DATE="20130617"
HALF_SYNTHESIS="50"
BANDS_MAPPING="bands_mapping_spot.txt"

#end of USER modif

if [ $# -lt 2 ]
then
  echo "Usage: $0 <resolution> <out folder name> [combined] [allinone]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

./run_composite.sh "$inputXML" $1 $2


