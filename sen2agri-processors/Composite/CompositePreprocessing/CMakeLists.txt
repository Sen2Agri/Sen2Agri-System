otb_create_application(
  NAME           CompositePreprocessing2
  SOURCES        CompositePreprocessing_2.cpp DirectionalCorrectionFunctor.h DirectionalCorrectionFunctor.txx DirectionalCorrection.cpp DirectionalModel.h DirectionalModel.cpp DirectionalCorrection.h DirectionalCorrectionFilter.h CreateS2AnglesRaster.h CreateS2AnglesRaster.cpp ComputeNDVI.h ComputeNDVI.cpp ResampleAtS2Res_2.cpp ResampleAtS2Res_2.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata MetadataHelper)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
