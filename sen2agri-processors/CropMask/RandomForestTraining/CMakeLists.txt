otb_create_application(
  NAME           RandomForestTraining
  SOURCES        RandomForestTraining.cpp ListSampleClassStatistics.hpp ShpFileSampleGeneratorFast.hpp ShpFileSampleGeneratorFast.txx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_RandomForestTraining DESTINATION usr/lib/otb/applications/)
