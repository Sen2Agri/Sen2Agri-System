#!/bin/bash

source set_build_folder.sh

./CropMaskFused.py \
    -refr /mnt/archive/reference_data/ESACCI-LC-L4-LCCS-Map-300m-P5Y-2010-v1.6.1.tif \
    -rseed 0 \
    -outdir /mnt/archive/ukraine/l4a-noinsitu/ \
    -strata /mnt/archive/strata/ukraine/UKr_Strata_All.shp \
    -input \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160407T042952_R107_V20160405T085012_20160405T085012.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160405.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160418T170647_R007_V20160418T090432_20160418T090432.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160418.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160428T165323_R007_V20160428T090022_20160428T090022.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160428.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160515T150946_R107_V20160515T085313_20160515T085313.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160515.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160607T235714_R007_V20160607T090406_20160607T090406.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160607.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160614T161101_R107_V20160614T085018_20160614T085018.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160614.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160618T074301_R007_V20160617T090020_20160617T090020.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160617.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160627T183540_R007_V20160627T090349_20160627T090349.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160627.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160708T173450_R107_V20160604T085207_20160604T085207.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160604.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160714T144734_R107_V20160714T085152_20160714T085152.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160714.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160717T201927_R007_V20160717T090142_20160717T090142.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160717.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160724T142347_R107_V20160724T085019_20160724T085019.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160724.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160727T143949_R007_V20160727T090022_20160727T090022.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160727.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160806T211343_R007_V20160806T090352_20160806T090352.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160806.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160823T203822_R107_V20160823T084602_20160823T084759.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160823.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160827T202223_R007_V20160826T085602_20160826T090349.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160826.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160903T183208_R107_V20160902T085012_20160902T085015.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160902.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160907T102838_R007_V20160905T090022_20160905T090017.SAFE/S2A_OPER_SSC_L2VALD_36UUU____20160905.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160407T042814_R107_V20160405T085012_20160405T085012.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160405.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160408T203002_R007_V20160408T090331_20160408T090331.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160408.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160418T171217_R007_V20160418T090432_20160418T090432.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160418.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160425T181206_R107_V20160425T085427_20160425T085427.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160425.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160428T164510_R007_V20160428T090022_20160428T090022.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160428.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160501T172532_R050_V20160501T091509_20160501T091509.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160501.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160505T164147_R107_V20160505T085020_20160505T085606.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160505.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160511T211235_R050_V20160511T091029_20160511T091029.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160511.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160515T151626_R107_V20160515T085313_20160515T085313.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160515.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160526T194706_R050_V20160521T090903_20160521T090903.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160521.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160614T161606_R107_V20160614T085018_20160614T085018.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160614.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160614T223347_R050_V20160610T090901_20160610T090901.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160610.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160618T074647_R007_V20160617T090020_20160617T090020.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160617.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160624T154040_R107_V20160624T085149_20160624T085149.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160624.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160627T184155_R007_V20160627T090349_20160627T090349.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160627.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160702T003141_R050_V20160630T090931_20160630T090931.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160630.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160708T173124_R107_V20160604T085207_20160604T085207.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160604.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160714T145323_R107_V20160714T085152_20160714T085152.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160714.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160717T202638_R007_V20160717T090142_20160717T090142.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160717.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160724T142241_R107_V20160724T085019_20160724T085019.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160724.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160727T143812_R007_V20160727T090022_20160727T090022.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160727.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160801T121025_R050_V20160720T090602_20160720T090851.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160720.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160806T210816_R007_V20160806T090352_20160806T090352.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160806.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160809T214922_R050_V20160809T091158_20160809T091158.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160809.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160823T204333_R107_V20160823T084602_20160823T084759.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160823.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160827T194030_R007_V20160826T085602_20160826T090349.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160826.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160830T190243_R050_V20160829T090552_20160829T090847.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160829.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160903T215723_R107_V20160902T085012_20160902T085015.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160902.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160907T101858_R007_V20160905T090022_20160905T090017.SAFE/S2A_OPER_SSC_L2VALD_36UUV____20160905.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160407T042952_R107_V20160405T085012_20160405T085012.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160405.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160418T170647_R007_V20160418T090432_20160418T090432.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160418.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160428T165323_R007_V20160428T090022_20160428T090022.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160428.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160515T150946_R107_V20160515T085313_20160515T085313.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160515.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160518T181818_R007_V20160518T090419_20160518T090419.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160518.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160525T153918_R107_V20160525T085021_20160525T085021.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160525.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160607T235714_R007_V20160607T090406_20160607T090406.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160607.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160614T161101_R107_V20160614T085018_20160614T085018.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160614.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160618T074301_R007_V20160617T090020_20160617T090020.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160617.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160624T152213_R107_V20160624T085149_20160624T085149.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160624.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160627T183540_R007_V20160627T090349_20160627T090349.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160627.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160708T173450_R107_V20160604T085207_20160604T085207.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160604.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160714T144734_R107_V20160714T085152_20160714T085152.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160714.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160717T201927_R007_V20160717T090142_20160717T090142.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160717.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160724T142347_R107_V20160724T085019_20160724T085019.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160724.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160727T143949_R007_V20160727T090022_20160727T090022.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160727.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160806T211343_R007_V20160806T090352_20160806T090352.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160806.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160823T203822_R107_V20160823T084602_20160823T084759.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160823.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160827T202223_R007_V20160826T085602_20160826T090349.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160826.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160903T183208_R107_V20160902T085012_20160902T085015.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160902.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160907T102838_R007_V20160905T090022_20160905T090017.SAFE/S2A_OPER_SSC_L2VALD_36UVU____20160905.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160407T042814_R107_V20160405T085012_20160405T085012.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160405.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160408T203002_R007_V20160408T090331_20160408T090331.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160408.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160418T171217_R007_V20160418T090432_20160418T090432.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160418.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160425T181206_R107_V20160425T085427_20160425T085427.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160425.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160428T164510_R007_V20160428T090022_20160428T090022.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160428.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160505T164147_R107_V20160505T085020_20160505T085606.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160505.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160515T151626_R107_V20160515T085313_20160515T085313.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160515.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160518T182555_R007_V20160518T090419_20160518T090419.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160518.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160607T234803_R007_V20160607T090406_20160607T090406.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160607.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160614T161606_R107_V20160614T085018_20160614T085018.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160614.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160618T074647_R007_V20160617T090020_20160617T090020.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160617.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160624T154040_R107_V20160624T085149_20160624T085149.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160624.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160627T184155_R007_V20160627T090349_20160627T090349.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160627.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160708T173124_R107_V20160604T085207_20160604T085207.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160604.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160714T145323_R107_V20160714T085152_20160714T085152.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160714.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160717T202638_R007_V20160717T090142_20160717T090142.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160717.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160724T142241_R107_V20160724T085019_20160724T085019.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160724.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160727T143812_R007_V20160727T090022_20160727T090022.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160727.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160806T210816_R007_V20160806T090352_20160806T090352.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160806.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160816T194830_R007_V20160816T090022_20160816T090444.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160816.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160823T204333_R107_V20160823T084602_20160823T084759.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160823.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160827T194030_R007_V20160826T085602_20160826T090349.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160826.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160903T215723_R107_V20160902T085012_20160902T085015.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160902.HDR \
    /mnt/archive/maccs_def_old/ukraine/l2a/S2A_OPER_PRD_MSIL2A_PDMC_20160907T101858_R007_V20160905T090022_20160905T090017.SAFE/S2A_OPER_SSC_L2VALD_36UVV____20160905.HDR \
    -buildfolder "$BUILD_FOLDER" # -mode validate
