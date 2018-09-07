find_package(Boost REQUIRED COMPONENTS filesystem regex)

set(Sen2AgriCommonUtils_HEADERS
    ../include/GlobalDefs.h
    ../include/BandsCfgMappingParser.h
    ../include/ResamplingBandExtractor.h
    ../include/ImageResampler.h
    ../include/GenericRSImageResampler.h)

set(Sen2AgriCommonUtils_SOURCES
    BandsCfgMappingParser.cpp)

add_library(Sen2AgriCommonUtils SHARED ${Sen2AgriCommonUtils_HEADERS} ${Sen2AgriCommonUtils_SOURCES})
target_link_libraries(Sen2AgriCommonUtils
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

target_include_directories(Sen2AgriCommonUtils PUBLIC ../include)

install(TARGETS Sen2AgriCommonUtils DESTINATION usr/lib/)
