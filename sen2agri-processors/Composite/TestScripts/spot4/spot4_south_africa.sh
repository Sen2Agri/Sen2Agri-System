MY_PWD=`pwd`

#USER modif
#add directories where SPOT products are to be found
inputXML1="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml "
inputXML1+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml "
inputXML1+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000.xml "
inputXML1+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000.xml "

inputXML2="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml "
inputXML2+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml "
inputXML2+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml "
inputXML2+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml "

inputXML3="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000.xml "
inputXML3+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000.xml "
inputXML3+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000.xml "
inputXML3+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000.xml "
inputXML3+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000.xml "

inputXML4="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000.xml "
inputXML4+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000.xml "
inputXML4+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000.xml "
inputXML4+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000.xml "
inputXML4+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000.xml "
inputXML4+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000.xml "

HALF_SYNTHESIS="15"

L3A_DATE_1="20130228"
L3A_DATE_2="20130328"
L3A_DATE_3="20130425"
L3A_DATE_4="20130523"

BANDS_MAPPING="bands_mapping_spot.txt"
#end of USER modif

if [ $# -lt 4 ]
then
  echo "Usage: $0 <otb directory application> <resolution> <out folder name> <bands mapping file> [scattering coefficient file - S2 case only]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

cd ../

SCAT_COEF=""
if [ $# == 5 ] ; then    
    ./run_composite.sh "$1" "$inputxml1 " "$2" "$3" "$L3A_DATE_1" "$HALF_SYNTHESIS" "$4" "$5"
    ./run_composite.sh "$1" "$inputxml2 " "$2" "$3" "$L3A_DATE_2" "$HALF_SYNTHESIS" "$4" "$5"
    ./run_composite.sh "$1" "$inputxml3 " "$2" "$3" "$L3A_DATE_3" "$HALF_SYNTHESIS" "$4" "$5"
    ./run_composite.sh "$1" "$inputxml4 " "$2" "$3" "$L3A_DATE_4" "$HALF_SYNTHESIS" "$4" "$5"
else
    ./run_composite.sh "$1" "$inputxml1 " "$2" "$3" "$L3A_DATE_1" "$HALF_SYNTHESIS" "$4"
    ./run_composite.sh "$1" "$inputxml2 " "$2" "$3" "$L3A_DATE_2" "$HALF_SYNTHESIS" "$4"
    ./run_composite.sh "$1" "$inputxml3 " "$2" "$3" "$L3A_DATE_3" "$HALF_SYNTHESIS" "$4"
    ./run_composite.sh "$1" "$inputxml4 " "$2" "$3" "$L3A_DATE_4" "$HALF_SYNTHESIS" "$4"
fi

cd $MY_PWD