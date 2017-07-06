otb_create_application(
    NAME           CropMaskStatistics
    SOURCES        CropMaskStatistics.cxx
                 ../Filters/CropMaskPreprocessing.h
    LINK_LIBRARIES ${OTB_LIBRARIES}
                   MACCSMetadata
                   TimeSeriesReader)

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropMaskStatistics OTBExtensions)

install(TARGETS otbapp_CropMaskStatistics DESTINATION usr/lib/otb/applications/)
