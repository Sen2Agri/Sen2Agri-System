set(MACCSMetadata_HEADERS
    ../include/FluentXML.hpp
    ../include/MACCSMetadataReader.hpp
    ../include/MACCSMetadataWriter.hpp
    ../include/MACCSMetadata.hpp)

set(MACCSMetadata_SOURCES
    FluentXML.cpp
    MACCSMetadataReader.cpp
    MACCSMetadataWriter.cpp)

add_library(MACCSMetadata SHARED ${MACCSMetadata_HEADERS} ${MACCSMetadata_SOURCES})
target_link_libraries(MACCSMetadata
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

target_include_directories(MACCSMetadata PUBLIC ../include)
