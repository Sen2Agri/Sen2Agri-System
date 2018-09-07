find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../Common/XmlUtils/include)

set(Sen2AgriPrdMetaWriters_HEADERS
    ../include/ProductMetadata.hpp
    ../include/ProductMetadataWriter.hpp
    ../include/TileMetadata.hpp
    ../include/TileMetadataWriter.hpp)

set(Sen2AgriPrdMetaWriters_SOURCES
    ProductMetadataWriter.cpp
    TileMetadataWriter.cpp)

add_library(Sen2AgriPrdMetaWriters SHARED ${Sen2AgriPrdMetaWriters_HEADERS} ${Sen2AgriPrdMetaWriters_SOURCES})
target_link_libraries(Sen2AgriPrdMetaWriters 
    Sen2AgriXmlUtils
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

target_include_directories(Sen2AgriPrdMetaWriters PUBLIC ../include)

install(TARGETS Sen2AgriPrdMetaWriters DESTINATION usr/lib/)
