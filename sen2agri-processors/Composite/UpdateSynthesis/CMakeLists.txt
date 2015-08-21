otb_create_application(
  NAME           UpdateSynthesis
  SOURCES        UpdateSynthesisFunctor.h UpdateSynthesisFunctor.txx UpdateSynthesis.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
