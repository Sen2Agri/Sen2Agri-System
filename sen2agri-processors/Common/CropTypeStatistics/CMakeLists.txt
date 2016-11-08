otb_create_application(
    NAME           CropTypeStatistics
    SOURCES        CropTypeStatistics.cxx
                 ../Filters/CropTypePreprocessing.h
                 ../Filters/TimeSeriesReader.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropTypeStatistics OTBExtensions)

install(TARGETS otbapp_CropTypeStatistics DESTINATION usr/lib/otb/applications/)
