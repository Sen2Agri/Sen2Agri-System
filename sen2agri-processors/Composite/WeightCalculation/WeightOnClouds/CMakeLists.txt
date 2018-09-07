otb_create_application(
  NAME           WeightOnClouds
  SOURCES        cloudmaskbinarization.h
                 cloudsinterpolation.h
                 gaussianfilter.h
                 cloudweightcomputation.h
                 paddingimagehandler.h
                 cuttingimagehandler.h
                 weightoncloudsapp.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_WeightOnClouds DESTINATION usr/lib/otb/applications/)

