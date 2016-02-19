find_package(Boost REQUIRED COMPONENTS filesystem)

otb_create_application(NAME DownloadSWBDTiles
                       SOURCES otbDownloadSWBDTiles.cxx
                       LINK_LIBRARIES OTBCommon OTBCurlAdapters "${Boost_LIBRARIES}")

install(TARGETS otbapp_DownloadSWBDTiles DESTINATION usr/lib/otb/applications/)
