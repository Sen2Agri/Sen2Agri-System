otb_create_application(
	NAME           StandardDeviation
	SOURCES        StandardDeviation.cpp
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_StandardDeviation DESTINATION usr/lib/otb/applications/)
