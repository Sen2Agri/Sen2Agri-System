otb_create_application(
  NAME           Trimming
  SOURCES        Trimming.cpp MahalanobisTrimmingFilter.h MahalanobisTrimmingFilter.hxx MahalanobisPixelExtractor.h MahalanobisPixelExtractor.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
