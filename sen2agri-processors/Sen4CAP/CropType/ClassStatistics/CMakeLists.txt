otb_create_application(
	NAME           ClassStatistics
	SOURCES        ClassStatistics.cpp
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.h
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../../Common/Filters)

add_dependencies(otbapp_ClassStatistics OTBExtensions)

install(TARGETS otbapp_ClassStatistics DESTINATION usr/lib/otb/applications/)
