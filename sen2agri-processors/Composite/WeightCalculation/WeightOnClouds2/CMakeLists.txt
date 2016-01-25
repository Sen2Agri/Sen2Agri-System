otb_create_application(
  NAME           WeightOnClouds2
  SOURCES        cloudmaskbinarization.h
                 cloudsinterpolation.h
                 gaussianfilter.h
                 cloudweightcomputation.h
                 paddingimagehandler.h
                 cuttingimagehandler.h
                 weightoncloudsapp.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

