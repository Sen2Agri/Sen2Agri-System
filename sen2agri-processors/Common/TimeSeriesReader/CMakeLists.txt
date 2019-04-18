find_package(Boost REQUIRED COMPONENTS filesystem)

set(TimeSeriesReader_HEADERS
    TimeSeriesReader.h
    otbSentinelMaskFilter.h
    otbSpotMaskFilter.h)

set(TimeSeriesReader_SOURCES
    TimeSeriesReader.cpp
    otbSentinelMaskFilter.cpp
    otbSpotMaskFilter.cpp)

add_library(TimeSeriesReader STATIC ${TimeSeriesReader_HEADERS} ${TimeSeriesReader_SOURCES})

target_link_libraries(TimeSeriesReader
    Sen2AgriProductReaders
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

set_target_properties(TimeSeriesReader PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(TimeSeriesReader PUBLIC .)

include_directories(../../Common/OTBExtensions)
