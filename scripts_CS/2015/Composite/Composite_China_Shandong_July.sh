#!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150430 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150702_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150702_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150712_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150712_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150727_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150727_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150811_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150811_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150816_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150816_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150821_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150821_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150826_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150826_N2A_ShandongChinaD0000B0000.xml" \
--res 10 --t0 20150410 --tend 20150525 --outdir /data/s2agri/output/China/Shangdong/composite/Composite_China_July --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
