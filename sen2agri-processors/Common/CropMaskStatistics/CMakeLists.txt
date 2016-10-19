otb_create_application(
    NAME           CropMaskStatistics
    SOURCES        CropMaskStatistics.cxx
                 ../Filters/CropMaskPreprocessing.h
                 ../Filters/TimeSeriesReader.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropMaskStatistics OTBExtensions)

install(TARGETS otbapp_CropMaskStatistics DESTINATION usr/lib/otb/applications/)
