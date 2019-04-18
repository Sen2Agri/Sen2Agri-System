add_subdirectory(UpdateSynthesis)
add_subdirectory(WeightCalculation)
add_subdirectory(MaskHandler)
add_subdirectory(CompositeSplitter)
add_subdirectory(DirectionalCorrection)
add_subdirectory(CompositePreprocessing)
add_subdirectory(QuantifyImage)

install(FILES TestScripts/composite_processing.py 
              TestScripts/composite_processing_CS.py 
        DESTINATION usr/bin/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE 
        WORLD_READ WORLD_EXECUTE)

install(FILES TestScripts/bands_mapping_s2.txt 
              TestScripts/bands_mapping_s2_L8.txt 
              TestScripts/bands_mapping_L8.txt 
              TestScripts/bands_mapping_Spot4_L8.txt 
              TestScripts/bands_mapping_Spot5_L8.txt
              TestScripts/bands_mapping_spot5.txt 
              TestScripts/bands_mapping_spot4.txt 
              TestScripts/scattering_coeffs_10m.txt
              TestScripts/scattering_coeffs_20m.txt 
              TestScripts/scattering_coeffs_droy_10m.txt
              TestScripts/scattering_coeffs_droy_20m.txt
              composite.map 
        DESTINATION usr/share/sen2agri/)
