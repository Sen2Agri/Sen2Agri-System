otb_create_application(
	NAME           ComputeClassCounts
	SOURCES        ComputeClassCounts.cxx
		       otbStreamingHistogramImageFilter.h
		       otbStreamingHistogramImageFilter.hxx
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_ComputeClassCounts DESTINATION usr/lib/otb/applications/)
