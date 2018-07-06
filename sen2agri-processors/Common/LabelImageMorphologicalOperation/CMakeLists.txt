otb_create_application(
	NAME           LabelImageMorphologicalOperation
	SOURCES        LabelImageMorphologicalOperation.cxx
                       itkLabelErodeImageFilter.h
                       itkLabelErodeImageFilter.hxx
	LINK_LIBRARIES ${OTB_LIBRARIES}
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

#add_dependencies(otbapp_LabelImageMorphologicalOperation OTBExtensions)

install(TARGETS otbapp_LabelImageMorphologicalOperation DESTINATION usr/lib/otb/applications/)
