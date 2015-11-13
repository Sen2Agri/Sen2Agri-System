##!/bin/bash
#USER modif
#add directories where SPOT products are to be found
../../sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /home/ramona/sen2agri-processors-build \
--syntdate 20150728 --synthalf 25 --input \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150703_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150703_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150708_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150708_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150718_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150718_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150728_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150728_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150807_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150807_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150817_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150817_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150822_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150822_N2A_SouthAfricaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/SouthAfrica/SPOT5_HRG2_XS_20150827_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150827_N2A_SouthAfricaD0000B0000.xml " \
--res 10 --t0 20150703 --tend 20150827 --outdir /mnt/output/ramona/Composite_SouthAfrica_July --bandsmap /home/ramona/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt