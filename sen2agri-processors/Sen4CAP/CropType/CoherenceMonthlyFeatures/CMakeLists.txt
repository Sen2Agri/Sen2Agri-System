otb_create_application(
	NAME           CoherenceMonthlyFeatures
	SOURCES        CoherenceMonthlyFeatures.cpp
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CoherenceMonthlyFeatures DESTINATION usr/lib/otb/applications/)
