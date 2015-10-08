otb_create_application(
  NAME           CompositePreprocessing
  SOURCES        CompositePreprocessing.cpp ResampleAtS2Res.cpp DirectionalCorrectionFunctor.h DirectionalCorrectionFunctor.txx DirectionalCorrection.cpp DirectionalModel.h DirectionalModel.cpp DirectionalCorrection.h DirectionalCorrectionFilter.h ResampleAtS2Res.h CreateS2AnglesRaster.h CreateS2AnglesRaster.cpp ComputeNDVI.h ComputeNDVI.cpp ResampledBandExtractor.cpp ResampledBandExtractor.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata MetadataHelper)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
