#USER modif
#add directories where L8 products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130426_L8_171_079/SouthAfricaS2A_20130426_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130512_L8_171_079/SouthAfricaS2A_20130512_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130528_L8_171_079/SouthAfricaS2A_20130528_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130613_L8_171_079/SouthAfricaS2A_20130613_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130629_L8_171_079/SouthAfricaS2A_20130629_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130715_L8_171_079/SouthAfricaS2A_20130715_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130731_L8_171_079/SouthAfricaS2A_20130731_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130816_L8_171_079/SouthAfricaS2A_20130816_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130901_L8_171_079/SouthAfricaS2A_20130901_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20130917_L8_171_079/SouthAfricaS2A_20130917_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20131019_L8_171_079/SouthAfricaS2A_20131019_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20131104_L8_171_079/SouthAfricaS2A_20131104_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20131120_L8_171_079/SouthAfricaS2A_20131120_L8_171_079.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/South Africa/MACCS_Manual_Format/SouthAfricaS2A_20131222_L8_171_079/SouthAfricaS2A_20131222_L8_171_079.HDR "

L3A_DATE="20131222"
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

