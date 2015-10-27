#USER modif
#add directories where SPOT products are to be found
inputXML="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20130414_L8_199_030/SudouestS2A_20130414_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20130719_L8_199_030/SudouestS2A_20130719_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20130804_L8_199_030/SudouestS2A_20130804_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20130820_L8_199_030/SudouestS2A_20130820_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20130905_L8_199_030/SudouestS2A_20130905_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20131007_L8_199_030/SudouestS2A_20131007_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20131023_L8_199_030/SudouestS2A_20131023_L8_199_030.HDR "
inputXML+="/mnt/Sen2Agri_DataSets/L2A/Landsat8/Sudmipy-West/MACCS_ManualFormat/SudouestS2A_20131210_L8_199_030/SudouestS2A_20131210_L8_199_030.HDR "

L3A_DATE="20131210"
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

