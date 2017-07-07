otb_create_application(
  NAME           PrincipalComponentAnalysis
  SOURCES        PrincipalComponentAnalysis.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})

include_directories(../../Common/Filters)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_PrincipalComponentAnalysis DESTINATION usr/lib/otb/applications/)
