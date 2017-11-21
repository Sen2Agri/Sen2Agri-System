otb_create_application(
    NAME           CropTypeStatistics
    SOURCES        CropTypeStatistics.cxx
                 ../Filters/CropTypePreprocessing.h
    LINK_LIBRARIES ${OTB_LIBRARIES}
                   MACCSMetadata
                   TimeSeriesReader)

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropTypeStatistics OTBExtensions)

