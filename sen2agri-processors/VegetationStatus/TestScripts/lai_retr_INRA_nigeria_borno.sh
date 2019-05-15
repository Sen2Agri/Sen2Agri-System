#! /bin/bash

CUR_DATE=`date +%Y-%m-%d`

FOLDER_NAME="test_lai_${CUR_DATE}_2"

# the inlaimonodir is set to the outdir (also if not specified) but it can be used to 
# to specify another folder where existing L3B product were previously produced for the given xml files
./lai_retrieve_processing.py --input \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T044353_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PQT____20160606.HDR" \
--res 10 --outdir /mnt/archive/test/L3B/Nigeria_Borno/${FOLDER_NAME} --laiBandsCfg /usr/share/sen2agri/Lai_Bands_Cfgs_Belcam.cfg --useinra YES --fcover YES --fapar YES \
--siteid 19 
