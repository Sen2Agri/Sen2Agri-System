otb_create_application(
	NAME           Composite
	SOURCES        Composite.cpp
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_Composite DESTINATION usr/lib/otb/applications/)
