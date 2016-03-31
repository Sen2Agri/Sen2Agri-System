otb_create_application(
  NAME           MaskHandler
  SOURCES        MaskHandlerFunctor.h MaskHandler.cpp MaskExtractorFilter.hxx
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_MaskHandler DESTINATION usr/lib/otb/applications/)
