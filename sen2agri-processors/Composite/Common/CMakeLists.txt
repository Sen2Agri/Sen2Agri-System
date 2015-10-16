set(MetadataHelper_HEADERS
    MACCSMetadataHelper.h
    MetadataHelperFactory.h
    MetadataHelper.h
    Spot4MetadataHelper.h
    GlobalDefs.h
    BandsCfgMappingParser.h
    ResamplingBandExtractor.h)

set(MetadataHelper_SOURCES
    MetadataHelper.cpp
    MetadataHelperFactory.cpp
    Spot4MetadataHelper.cpp
    MACCSMetadataHelper.cpp
    BandsCfgMappingParser.cpp
    ResamplingBandExtractor.cpp)

add_library(MetadataHelper SHARED ${MetadataHelper_HEADERS} ${MetadataHelper_SOURCES})
target_link_libraries(MetadataHelper
    "${OTBCommon_LIBRARIES}"
    "${OTBITK_LIBRARIES}" MACCSMetadata)

target_include_directories(MetadataHelper PUBLIC .)

install(TARGETS MetadataHelper DESTINATION lib)
