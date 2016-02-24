add_subdirectory(SampleSelection)
add_subdirectory(BandsExtractor)
add_subdirectory(FeatureExtractionOld)
add_subdirectory(FeatureExtraction)
add_subdirectory(TemporalResampling)

install(FILES ../../scripts/CropType.py 
        DESTINATION usr/bin/)

