add_subdirectory(SampleSelection)
add_subdirectory(BandsExtractor)
add_subdirectory(FeatureExtractionOld)
add_subdirectory(FeatureExtraction)
add_subdirectory(TemporalResampling)

install(PROGRAMS ../../scripts/CropType.py
        DESTINATION bin/)
#        DESTINATION bin/python/)

install(FILES crop-type.lut
        DESTINATION share/sen2agri)
