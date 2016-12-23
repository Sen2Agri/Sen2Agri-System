#! /bin/bash

CUR_DATE=`date +%Y-%m-%d`
FOLDER_NAME="pheno_${CUR_DATE}"

#add directories where SPOT products are to be found
./pheno_processing.py --input \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160607T134539_R136_V20160606T094258_20160606T094258.SAFE/S2A_OPER_SSC_L2VALD_32PRS____20160606.HDR" \
"/mnt/archive/test/Borno_South/S2A_OPER_PRD_MSIL2A_PDMC_20160817T185332_R136_V20160815T093042_20160815T094108.SAFE/S2A_OPER_SSC_L2VALD_32PRS____20160815.HDR" \
--siteid 19 --resolution 10 --outdir /mnt/archive/test/L3B/Nigeria_Borno/${FOLDER_NAME}
#end of USER modif
