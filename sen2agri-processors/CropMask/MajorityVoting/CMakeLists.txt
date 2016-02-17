otb_create_application(
  NAME           MajorityVoting
  SOURCES        otbClassLabelImageRegularization.cxx otbLabelMapWithMajorityClassLabelFilter.h otbLabelMapWithMajorityClassLabelFilter.txx 
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_MajorityVoting DESTINATION usr/lib/otb/applications/)
