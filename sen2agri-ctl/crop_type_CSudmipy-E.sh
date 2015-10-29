#!/bin/bash
./sen2agri-ctl.py submit-job -s Belgium crop-type -i \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130217_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130222_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130227_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130304_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130319_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130324_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130329_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130403_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130413_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130418_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130423_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130503_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130513_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130518_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130523_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130602_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130607_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130612_N2A_CSudmipy-ED0000B0000/ \
/mnt/Satellite_Imagery/S2-QR/CSudmipy-E-for-crop_LEVEL2A/SPOT4_HRVIR1_XS_20130617_N2A_CSudmipy-ED0000B0000/ \
-r /mnt/Imagery_S2A/In-Situ_TDS/France/LC/SudmipyS2A_LandCoverDecoupe_dissolvedGeometry.shp \
--resolution 20 --date-start 20130217 --date-end 20130617 -p crop-type.sampling-rate 5
