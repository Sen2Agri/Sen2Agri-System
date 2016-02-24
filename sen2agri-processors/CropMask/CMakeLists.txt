add_subdirectory(RandomSelection)
add_subdirectory(DataSmoothing)
add_subdirectory(TemporalFeaturesNoInsitu)
add_subdirectory(PrincipalComponentAnalysis)
add_subdirectory(MeanShiftSegmentation)
add_subdirectory(MajorityVoting)
add_subdirectory(RandomForestTraining)
add_subdirectory(Trimming)
add_subdirectory(Erosion)
add_subdirectory(FeaturesWithInsitu)
add_subdirectory(FeaturesWithoutInsitu)

install(FILES ../../scripts/CropMask.py  
        DESTINATION usr/bin/)
