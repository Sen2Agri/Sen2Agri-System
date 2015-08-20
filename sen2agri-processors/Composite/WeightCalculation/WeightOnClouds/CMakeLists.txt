otb_create_application(
  NAME           WeightOnClouds
  SOURCES        cloudmaskbinarization.cpp cloudsinterpolation.cpp cloudweightcomputation.cpp gaussianfilter.cpp weightoncloudsapp.cpp
  LINK_LIBRARIES MACCSMetadata ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

