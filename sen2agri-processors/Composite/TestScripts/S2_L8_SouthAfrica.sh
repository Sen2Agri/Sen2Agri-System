#USER modif

if [ "$#" -eq 3 ]; then
    BUILD_FOLDER=$1
    RES=$2
    OUT_FOLDER=$3
    APP_LOCATION="--applocation $BUILD_FOLDER"
else
#    BUILD_FOLDER="~/sen2agri-processors-build"
    BUILD_FOLDER=""
    APP_LOCATION=""
    RES=10
    CUR_DATE=`date +%Y-%m-%d`
    OUT_FOLDER="/mnt/archive/l3a/composite_${CUR_DATE}"
fi

./composite_processing.py $APP_LOCATION --syntdate 20160101 --synthalf 25 --input \
"/mnt/archive/maccs/south_africa/l2a/LC81710792015346LGN00_L2A/L8_TEST_L8C_L2VALD_171079_20151212.HDR" \
"/mnt/archive/maccs/south_africa/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160108T173225_R135_V20160108T082023_20160108T082023.SAFE/S2A_OPER_SSC_L2VALD_35JMK____20160108.HDR" \
"/mnt/archive/maccs/south_africa/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160114T023112_R035_V20160111T082928_20160111T082928.SAFE/S2A_OPER_SSC_L2VALD_35JMK____20160111.HDR" \
--siteid 03 --res $RES --outdir "$OUT_FOLDER/20160101/35JMK" --bandsmap /usr/share/sen2agri/bands_mapping_s2_L8.txt --scatteringcoef /usr/share/sen2agri/scattering_coeffs_10m.txt

./composite_processing.py $APP_LOCATION --syntdate 20160101 --synthalf 25 --input \
"/mnt/archive/maccs/south_africa/l2a/LC81710792015346LGN00_L2A/L8_TEST_L8C_L2VALD_171079_20151212.HDR" \
"/mnt/archive/maccs/south_africa/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160108T173225_R135_V20160108T082023_20160108T082023.SAFE/S2A_OPER_SSC_L2VALD_35JNK____20160108.HDR" \
"/mnt/archive/maccs/south_africa/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160114T023112_R035_V20160111T082928_20160111T082928.SAFE/S2A_OPER_SSC_L2VALD_35JNK____20160111.HDR" \
--siteid 03 --res $RES --outdir "$OUT_FOLDER/20160101/35JNK" --bandsmap /usr/share/sen2agri/bands_mapping_s2_L8.txt --scatteringcoef /usr/share/sen2agri/scattering_coeffs_10m.txt
