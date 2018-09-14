otb_create_application(
  NAME           DirectionalCorrection
  SOURCES        DirectionalCorrectionFunctor.h DirectionalCorrectionFunctor.txx DirectionalCorrection.cpp DirectionalModel.h DirectionalModel.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
