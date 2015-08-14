set(MACCSMetadata_HEADERS
    ../include/FluentXML.hpp
    ../include/tinyxml_utils.hpp
    ../include/MACCSMetadata.hpp
    ../include/SPOT4Metadata.hpp
    ../include/MACCSMetadataReader.hpp
    ../include/MACCSMetadataWriter.hpp
    ../include/SPOT4MetadataReader.hpp)

set(MACCSMetadata_SOURCES
    FluentXML.cpp
    tinyxml_utils.cpp
    MACCSMetadataReader.cpp
    MACCSMetadataWriter.cpp
    SPOT4MetadataReader.cpp)

add_library(MACCSMetadata SHARED ${MACCSMetadata_HEADERS} ${MACCSMetadata_SOURCES})
target_link_libraries(MACCSMetadata
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

target_include_directories(MACCSMetadata PUBLIC ../include)

install(TARGETS MACCSMetadata DESTINATION lib)
