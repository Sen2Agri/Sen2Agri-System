#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000.xml "
#inputXML+="/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000.xml "

#L3A_DATE="20130616"
L3A_DATE="20130407"
#HALF_SYNTHESIS="50"
HALF_SYNTHESIS="25"
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
    ./run_composite.sh "$1" "$inputxml " "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$4" "$5"
else
    ./run_composite.sh "$1" "$inputxml " "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$4"
fi

