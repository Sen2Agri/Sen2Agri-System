otb_create_application(NAME DownloadSWBDTiles
                       SOURCES otbDownloadSWBDTiles.cxx
                       LINK_LIBRARIES OTBCommon OTBCurlAdapters)

install(TARGETS otbapp_DownloadSWBDTiles DESTINATION lib/otb/applications/)
