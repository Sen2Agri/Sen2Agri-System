otb_create_application(
  NAME           UpdateSynthesis
  SOURCES        UpdateSynthesisFunctor.h UpdateSynthesis.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
