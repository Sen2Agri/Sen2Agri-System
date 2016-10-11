otb_create_application(
    NAME           MultiModelImageClassifier
    SOURCES        otbMultiModelImageClassifier.cxx
                   otbMultiModelImageClassificationFilter.h
                   otbMultiModelImageClassificationFilter.txx
  LINK_LIBRARIES ${OTB_LIBRARIES})

install(TARGETS otbapp_MultiModelImageClassifier DESTINATION usr/lib/otb/applications/)
