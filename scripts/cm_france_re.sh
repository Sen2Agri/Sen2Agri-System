#!/bin/sh
source ./set_build_folder.sh

./CropMaskFused.py \
-outdir \
/mnt/data/france \
-targetfolder \
/mnt/archive/france/l4a/ \
-red-edge \
-refp \
/mnt/data/france/FR_MP_LC_FO_20160712_0728-0901-0411.shp \
-input \
/mnt/archive/maccs_def/france/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160209T183641_R094_V20160204T110240_20160204T110240.SAFE/S2A_OPER_SSC_L2VALD_30TYN____20160204.HDR \
/mnt/archive/maccs_def/france/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160408T230425_R094_V20160214T110150_20160214T110150.SAFE/S2A_OPER_SSC_L2VALD_30TYN____20160214.HDR \
/mnt/archive/maccs_def/france/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160312T234322_R051_V20160312T105037_20160312T105037.SAFE/S2A_OPER_SSC_L2VALD_30TYN____20160312.HDR \
-buildfolder "$BUILD_FOLDER"
