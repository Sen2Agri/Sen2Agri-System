include_directories(../include)

FILE(GLOB_RECURSE MACCSMetadata_HEADERS ../include/*.hpp)
set(MACCSMetadata_SOURCES FluentXML.cpp MACCSMetadataReader.cpp MACCSMetadataWriter.cpp)

add_library(MACCSMetadata SHARED ${MACCSMetadata_HEADERS} ${MACCSMetadata_SOURCES})
target_link_libraries(MACCSMetadata
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")
