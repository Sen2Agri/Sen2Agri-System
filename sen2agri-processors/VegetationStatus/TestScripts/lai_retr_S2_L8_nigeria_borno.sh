#! /bin/bash

CUR_DATE=`date +%Y-%m-%d`

FOLDER_NAME="test_lai_${CUR_DATE}_2"

# the inlaimonodir is set to the outdir (also if not specified) but it can be used to 
# to specify another folder where existing L3B product were previously produced for the given xml files
./lai_retrieve_processing.py --input \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T044353_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PQT____20160606.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T044353_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PRT____20160606.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T134539_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PQS____20160606.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T134539_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PRS____20160606.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160817T185332_R136_V20160815T093042_20160815T094108.SAFE/S2A_OPER_SSC_L2VALD_32PQS____20160815.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160817T185332_R136_V20160815T093042_20160815T094108.SAFE/S2A_OPER_SSC_L2VALD_32PRS____20160815.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160817T214327_R136_V20160815T093042_20160815T094108.SAFE/S2A_OPER_SSC_L2VALD_32PQT____20160815.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160817T214327_R136_V20160815T093042_20160815T094108.SAFE/S2A_OPER_SSC_L2VALD_32PRT____20160815.HDR" \
"/mnt/archive/test/Borno_South/LC81860522016182LGN00_L2A/L8_TEST_L8C_L2VALD_186052_20160630.HDR" \
"/mnt/archive/test/Borno_South/LC81860532016182LGN00_L2A/L8_TEST_L8C_L2VALD_186053_20160630.HDR" \
--res 0 --outdir /mnt/archive/test/L3B/Nigeria_Borno/${FOLDER_NAME} --rsrcfg /usr/share/sen2agri/rsr_cfg.txt --modelsfolder /mnt/archive/L3B_GeneratedModels_Test/ \
--siteid 19 --generatemodel YES --useintermlaifiles NO --generatemonodate YES --genreprocessedlai NO --genfittedlai NO --inlaimonodir /mnt/archive/test/L3B/Nigeria_Borno/${FOLDER_NAME}
