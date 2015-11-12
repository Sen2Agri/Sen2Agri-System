##!/bin/bash
#USER modif
#add directories where SPOT products are to be found
../../sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /home/ramona/sen2agri-processors-build \
--syntdate 20150525 --synthalf 25 --input \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150505_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150505_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150510_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150510_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150515_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150515_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150520_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150520_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150525_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150525_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150604_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150604_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150614_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150614_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150624_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150624_N2A_BurkinaD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Burkina/SPOT5_HRG2_XS_20150629_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150629_N2A_BurkinaD0000B0000.xml " \
--res 10 --t0 20150505 --tend 20150629 --outdir /mnt/output/ramona/Composite_Burkina_May --bandsmap /home/ramona/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt