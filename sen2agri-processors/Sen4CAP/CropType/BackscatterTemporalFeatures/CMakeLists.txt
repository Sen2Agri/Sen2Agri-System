otb_create_application(
	NAME           BackscatterTemporalFeatures
	SOURCES        BackscatterTemporalFeatures.cpp
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_BackscatterTemporalFeatures DESTINATION usr/lib/otb/applications/)
