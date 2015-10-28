#USER modif
#add directories where L8 products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130416_L8_181_025/EUkraineS2A_20130416_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130502_L8_181_025/EUkraineS2A_20130502_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130518_L8_181_025/EUkraineS2A_20130518_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130603_L8_181_025/EUkraineS2A_20130603_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130619_L8_181_025/EUkraineS2A_20130619_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130705_L8_181_025/EUkraineS2A_20130705_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130721_L8_181_025/EUkraineS2A_20130721_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130806_L8_181_025/EUkraineS2A_20130806_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130822_L8_181_025/EUkraineS2A_20130822_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20131110_L8_181_025/EUkraineS2A_20131110_L8_181_025.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20131228_L8_181_025/EUkraineS2A_20131228_L8_181_025.HDR "

L3A_DATE="20131228"
HALF_SYNTHESIS="120"
BANDS_MAPPING="bands_mapping_L8.txt"
#end of USER modif

if [ $# -lt 4 ]
then
  echo "Usage: $0 <otb directory application> <resolution> <out folder name> <bands mapping file> [scattering coefficient file - S2 case only]"
  echo "The file with input xmls should be given. The resolution for which the computations will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

SCAT_COEF=""
if [ $# == 5 ] ; then    
    ./run_composite.sh "$1" "$inputXML" "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$4" "$5"
else
    ./run_composite.sh "$1" "$inputXML" "$2" "$3" "$L3A_DATE" "$HALF_SYNTHESIS" "$4"
fi

