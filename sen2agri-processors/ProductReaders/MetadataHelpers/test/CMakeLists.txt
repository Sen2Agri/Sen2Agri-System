otb_create_application(
  NAME           MetadataHelpersApp
  SOURCES        MetadataHelpersApp.cpp
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} Sen2AgriProductReaders)

install(TARGETS otbapp_MetadataHelpersApp DESTINATION usr/lib/otb/applications/)

