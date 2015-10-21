otb_create_application(
  NAME           Trimming
  SOURCES        Trimming.cpp MahalanobisTrimmingFilter.h MahalanobisTrimmingFilter.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_Trimming DESTINATION lib/otb/applications/)
