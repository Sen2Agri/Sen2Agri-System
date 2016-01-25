otb_create_application(
  NAME           WeightOnClouds2
  SOURCES        cloudmaskbinarization.h
                 cloudsinterpolation.h
                 gaussianfilter.h
                 cloudweightcomputation.cpp
                 weightoncloudsapp.cpp
                 paddingimagehandler.h
                 cuttingimagehandler.h
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

