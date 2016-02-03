#! /bin/bash -l

python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--input \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/BelgiumS2A_20130728_L8_198_025_USGS_surf_pente_30m/BelgiumS2A_20130728_L8_198_025_USGS_surf_pente_30m.hdr" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/BelgiumS2A_20130930_L8_198_025_USGS_surf_pente_30m/BelgiumS2A_20130930_L8_198_025_USGS_surf_pente_30m.hdr" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/BelgiumS2A_20131203_L8_198_025_USGS_surf_pente_30m/BelgiumS2A_20131203_L8_198_025_USGS_surf_pente_30m.hdr" \
--res 0 \
--t0 20130101 \
--tend 20130101 \
--generatemodel YES \
--outdir /data/s2agri/output/2013/Belgium/LAI_Belgium_monodate_L8 \
--rsrfile /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr
