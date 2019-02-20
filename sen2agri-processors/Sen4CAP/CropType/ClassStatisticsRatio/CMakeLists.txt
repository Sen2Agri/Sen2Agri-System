otb_create_application(
	NAME           ClassStatisticsRatio
	SOURCES        ClassStatisticsRatio.cpp
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.h
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.hxx
                 ../../../Common/Filters/RatioFilterVector.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../../Common/Filters)

add_dependencies(otbapp_ClassStatisticsRatio OTBExtensions)

install(TARGETS otbapp_ClassStatisticsRatio DESTINATION usr/lib/otb/applications/)
