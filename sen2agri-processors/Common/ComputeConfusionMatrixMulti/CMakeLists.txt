otb_create_application(
  NAME           ComputeConfusionMatrixMulti
  SOURCES        otbComputeConfusionMatrixMulti.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_ComputeConfusionMatrixMulti DESTINATION usr/lib/otb/applications/)
