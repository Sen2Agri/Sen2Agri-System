#USER modif
#add directories where SPOT products are to be found
./composite_processing.py --applocation /home/agrosu/sen2agri-processors-build --syntdate 20130228 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130216_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130221_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml" \
--res 20 --t0 20130216 --tend 20130323 --outdir /mnt/scratch/composite_python_midp_W/20130228 --bandsmap /home/agrosu/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt
./composite_processing.py --applocation /home/agrosu/sen2agri-processors-build --syntdate 20130328 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130303_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130308_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130318_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130323_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130407_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml" \
--res 20 --t0 20130303 --tend 20130422 --outdir /mnt/scratch/composite_python_midp_W/20130328 --bandsmap /home/agrosu/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt
./composite_processing.py --applocation /home/agrosu/sen2agri-processors-build --syntdate 20130425 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130412_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130417_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130422_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml" \
--res 20 --t0 20130412 --tend 20130517 --outdir /mnt/scratch/composite_python_midp_W/20130425 --bandsmap /home/agrosu/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt
./composite_processing.py --applocation /home/agrosu/sen2agri-processors-build --syntdate 20130523 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130512_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130517_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130527_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130606_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130611_N2A_CSudmipy-OD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000/SPOT4_HRVIR_XS_20130616_N2A_CSudmipy-OD0000B0000.xml" \
--res 20 --t0 20130512 --tend 20130616 --outdir /mnt/scratch/composite_python_midp_W/20130523 --bandsmap /home/agrosu/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt
