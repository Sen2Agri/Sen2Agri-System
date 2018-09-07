find_package(Boost REQUIRED COMPONENTS filesystem regex)

set(Sen2AgriProductReaders_HEADERS
    MACCSMetadataHelper.h
    ../include/MetadataHelperFactory.h
    ../include/MetadataHelper.h
    Spot4MetadataHelper.h)

set(Sen2AgriProductReaders_SOURCES
    MetadataHelper.cpp
    MetadataHelperFactory.cpp
    Spot4MetadataHelper.cpp
    MACCSMetadataHelper.cpp)

add_library(Sen2AgriProductReaders SHARED ${Sen2AgriProductReaders_HEADERS} ${Sen2AgriProductReaders_SOURCES})
target_link_libraries(Sen2AgriProductReaders
    Sen2AgriCommonUtils
    MACCSMetadata
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

target_include_directories(Sen2AgriProductReaders PUBLIC ../include)

install(TARGETS Sen2AgriProductReaders DESTINATION usr/lib/)
