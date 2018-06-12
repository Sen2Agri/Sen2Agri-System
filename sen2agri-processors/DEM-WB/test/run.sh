#!/bin/sh
# ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_l8 /mnt/Sen2Agri_DataSets/L2A/MACCS/L1/L8/LC81980302013177LGN01/LC81980302013177LGN01_B1.TIF /mnt/scratch/dem_l8
# ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_s2 /mnt/Sen2Agri_DataSets/L2A/MACCS/L1/S2/S2A_OPER_PRD_MSIL1C_PDMC_20150428T110328_R065_V20091211T165851_20091211T165932.SAFE/GRANULE/S2A_OPER_MSI_L1C_TL_MPS__20150428T110328_A000065_T15SVD_N01.01/IMG_DATA/S2A_OPER_MSI_L1C_TL_MPS__20150428T110328_A000065_T15SVD_B02.jp2 /mnt/scratch/dem_s2
# ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_s2 /mnt/output/L1C/S2A/SouthAfrica/S2A_OPER_PRD_MSIL1C_PDMC_20151003T144413_R035_V20151003T083038_20151003T083038.SAFE/GRANULE/S2A_OPER_MSI_L1C_TL_SGS__20151003T114927_A001462_T35JME_N01.04/IMG_DATA/S2A_OPER_MSI_L1C_TL_SGS__20151003T114927_A001462_T35JME_B02.jp2 /mnt/scratch/dem_s2
# ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_s2 /mnt/output/L1C/S2A/S2A_OPER_PRD_MSIL1C_PDMC_20151207T225114_R111_V20151207T155104_20151207T155104.SAFE/GRANULE/S2A_OPER_MSI_L1C_TL_MTI__20151207T203600_A002396_T17NNJ_N02.00/IMG_DATA/S2A_OPER_MSI_L1C_TL_MTI__20151207T203600_A002396_T17NNJ_B02.jp2 /mnt/scratch/dem_s2
# for file in $(find /mnt/output/L1C/S2A/S2A_OPER_PRD_MSIL1C_PDMC_20151207T225114_R111_V20151207T155104_20151207T155104.SAFE/GRANULE/ -name "*_B02.jp2"); do
#     ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_s2 "$file" /mnt/scratch/dem_s2
# done

for file in $(find /mnt/data/sentinel_2_dwn_ukraine/S2A_OPER_PRD_MSIL1C_PDMC_20151216T142647_R093_V20151216T093231_20151216T093231.SAFE/GRANULE -name "*_B02.jp2"); do
    ./dem.py --srtm /mnt/data/srtm --swbd /mnt/data/swbd -w /mnt/scratch/tmp_s2 "$file" /mnt/scratch/dem_s2
done
wait
