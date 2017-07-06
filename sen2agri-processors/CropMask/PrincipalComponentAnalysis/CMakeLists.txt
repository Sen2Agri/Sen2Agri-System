otb_create_application(
  NAME           PrincipalComponentAnalysis
  SOURCES        PrincipalComponentAnalysis.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_PrincipalComponentAnalysis DESTINATION usr/lib/otb/applications/)
