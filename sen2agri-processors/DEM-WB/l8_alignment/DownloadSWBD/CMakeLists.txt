cmake_minimum_required(VERSION 2.8.3)

find_package(OTB REQUIRED)
IF(OTB_FOUND)
    INCLUDE(${OTB_USE_FILE})
ELSE(OTB_FOUND)
    MESSAGE(FATAL_ERROR
    "Cannot build OTB project without OTB. Please set OTB_DIR.")
ENDIF(OTB_FOUND)

OTB_CREATE_APPLICATION(NAME DownloadSWBDTiles 
                       SOURCES otbDownloadSWBDTiles.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon)


