otb_create_application(
  NAME           UpdateSynthesis
  SOURCES        UpdateSynthesisFunctor_2.h UpdateSynthesisFunctor_2.txx UpdateSynthesis_2.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_UpdateSynthesis DESTINATION usr/lib/otb/applications/)
