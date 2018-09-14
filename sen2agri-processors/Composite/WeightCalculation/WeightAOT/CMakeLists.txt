otb_create_application(
  NAME           WeightAOT
  SOURCES        weightonaot.cpp weightaotapp.cpp
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_WeightAOT DESTINATION usr/lib/otb/applications/)
