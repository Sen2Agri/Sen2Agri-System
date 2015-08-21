otb_create_application(
  NAME           SpotMaskHandler
  SOURCES        SpotMaskHandlerFunctor.h SpotMaskHandler.cpp SpotMaskExtractorFilter.hxx
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
