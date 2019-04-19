add_subdirectory(SampleSelection)
#add_subdirectory(BandsExtractor)
add_subdirectory(FeatureExtraction)
add_subdirectory(QualityFlagsExtractor)

install(FILES ../../scripts/CropType.py ../../scripts/CropTypeFused.py
        DESTINATION usr/bin/
	    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES crop-type.lut
        DESTINATION usr/share/sen2agri)
