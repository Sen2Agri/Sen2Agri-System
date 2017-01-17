otb_create_application(
    NAME           FillNoData
    SOURCES        FillNoData.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

include_directories(../../Common/Filters)

install(TARGETS otbapp_FillNoData DESTINATION usr/lib/otb/applications/)
