cmake_minimum_required(VERSION 2.8)

set(GAPFILLING_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/temporalgapfilling/src/library)
set(PHENOTB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/phenotb/src/Library)
set(PHENOTB_LIBRARY OTBPheno)

add_subdirectory(otb-bv/src)
add_subdirectory(phenotb/src)
add_subdirectory(temporalgapfilling/src)

install(FILES TestScripts/lai_retrieve_processing.py
              TestScripts/pheno_processing.py
        DESTINATION usr/bin/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES lai.map
              LaiCommonBVDistributionSamples.txt
        DESTINATION usr/share/sen2agri)

install(FILES ConfigFiles/rsr_cfg.txt
              ConfigFiles/Lai_Bands_Cfgs.cfg
              ConfigFiles/Lai_Bands_S2.cfg
              ConfigFiles/Lai_Bands_L8.cfg
              ConfigFiles/Lai_Bands_Cfgs_Belcam.cfg
              ConfigFiles/Lai_Bands_S2_Belcam_rededge.cfg
              ConfigFiles/Lai_Bands_S2_Belcam_allbands.cfg
              ConfigFiles/Lai_Bands_L8_Belcam.cfg
              ConfigFiles/Algo_S2_V2.0_SL2T_rededge_LAI.txt
              ConfigFiles/Algo_S2_V2.0_SL2T_rededge_FAPAR.txt
              ConfigFiles/Algo_S2_V2.0_SL2T_rededge_FCOVER.txt
              ConfigFiles/Algo_S2_V2.0_SL2T_allbands_LAI.txt
              ConfigFiles/Algo_S2_V2.0_SL2T_allbands_FAPAR.txt
              ConfigFiles/Algo_S2_V2.0_SL2T_allbands_FCOVER.txt
              ConfigFiles/Algo_LANDSAT8OLI_LAI.txt
              ConfigFiles/Algo_LANDSAT8OLI_FAPAR.txt
              ConfigFiles/Algo_LANDSAT8OLI_FCOVER.txt
              otb-bv/data/landsat8_7b.rsr
              otb-bv/data/Sentinel2_10m.rsr
              otb-bv/data/Sentinel2_10m_real.rsr
              otb-bv/data/Sentinel2_10m_theoretical.rsr
              otb-bv/data/Sentinel2_real.rsr
              otb-bv/data/spot4hrvir1.rsr
              otb-bv/data/spot5hrg1.rsr
        DESTINATION usr/share/sen2agri/)
