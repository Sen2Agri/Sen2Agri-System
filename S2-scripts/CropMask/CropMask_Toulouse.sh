#!/bin/bash


source set_build_folder.sh

../../scripts/CropMask.py -refr /mnt/data/reference/Reference_SudMiPy_rep.tif -ratio 0.75 -input \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150706/S2A_OPER_SSC_L2VALD_31TCJ____20150706.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150716/S2A_OPER_SSC_L2VALD_31TCJ____20150716.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150726/S2A_OPER_SSC_L2VALD_31TCJ____20150726.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150805/S2A_OPER_SSC_L2VALD_31TCJ____20150805.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150825/S2A_OPER_SSC_L2VALD_31TCJ____20150825.HDR \
-t0 20150706 -tend 20150825 -rate 10 -radius 100 -nbtrsample 4000 -rseed 0 -window 2 -lmbd 2 -weight 1 -nbcomp 6 -spatialr 10 -ranger 0.65 -minsize 10 -rfnbtrees 100 -rfmax 25 -rfmin 25 -eroderad 5 -alpha 0.01 -tilename T31TCJ -pixsize 10 -mission SENTINEL \
-outdir /mnt/data/toulouse/Toulouse-mask -targetfolder /mnt/output/L4A/Sentinel-2/Toulouse -buildfolder $BUILD_FOLDER
