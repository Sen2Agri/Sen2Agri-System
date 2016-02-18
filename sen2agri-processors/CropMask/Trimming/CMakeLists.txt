otb_create_application(
  NAME           Trimming
  SOURCES        Trimming.cpp MahalanobisTrimmingFilter.h MahalanobisTrimmingFilter.hxx MahalanobisTrimmingFilterThreaded.h MahalanobisTrimmingFilterThreaded.hxx MahalanobisPixelExtractor.h MahalanobisPixelExtractor.hxx MahalanobisTrimmingFilterOld.h MahalanobisTrimmingFilterOld.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_Trimming DESTINATION usr/lib/otb/applications/)
