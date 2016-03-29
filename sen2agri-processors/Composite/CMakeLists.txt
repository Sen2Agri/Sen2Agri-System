add_subdirectory(Common)
add_subdirectory(ComputeNDVI)
add_subdirectory(UpdateSynthesis)
add_subdirectory(WeightCalculation)
add_subdirectory(ResampleAtS2Res)
add_subdirectory(MaskHandler)
add_subdirectory(CompositeSplitter)
add_subdirectory(CreateS2AnglesRaster)
add_subdirectory(DirectionalCorrection)
add_subdirectory(CompositePreprocessing)
add_subdirectory(QuantifyImage)

install(PROGRAMS TestScripts/composite_processing.py 
        DESTINATION bin/)
#        DESTINATION bin/python)

install(FILES TestScripts/bands_mapping_s2.txt 
              TestScripts/bands_mapping_Spot4_L8.txt 
              TestScripts/bands_mapping_spot5.txt 
              TestScripts/bands_mapping_spot.txt 
              TestScripts/scattering_coeffs_10m.txt
              TestScripts/scattering_coeffs_20m.txt 
        DESTINATION share/sen2agri/)

