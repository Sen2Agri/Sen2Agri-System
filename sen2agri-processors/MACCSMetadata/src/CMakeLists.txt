find_package(Boost REQUIRED COMPONENTS filesystem)

set(MACCSMetadata_HEADERS
    ../include/FluentXML.hpp
    ../include/tinyxml_utils.hpp
    ../include/string_utils.hpp
    ../include/MACCSMetadata.hpp
    ../include/TileMetadata.hpp
    ../include/ProductMetadata.hpp
    ../include/SPOT4Metadata.hpp
    ../include/MACCSMetadataReader.hpp
    ../include/MACCSMetadataWriter.hpp
    ../include/SPOT4MetadataReader.hpp
    ../include/TileMetadataWriter.hpp
    ../include/ProductMetadataWriter.hpp
    ../include/ViewingAngles.hpp
    ../include/MetadataUtil.hpp)

set(MACCSMetadata_SOURCES
    FluentXML.cpp
    tinyxml_utils.cpp
    string_utils.cpp
    MACCSMetadataReader.cpp
    MACCSMetadataWriter.cpp
    TileMetadataWriter.cpp
    ProductMetadataWriter.cpp
    SPOT4MetadataReader.cpp
    ViewingAngles.cpp
    MetadataUtil.cpp)

add_library(MACCSMetadata SHARED ${MACCSMetadata_HEADERS} ${MACCSMetadata_SOURCES})
target_link_libraries(MACCSMetadata
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

otb_create_application(
  NAME           ProductFormatter
  SOURCES        ProductFormatter.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

target_include_directories(MACCSMetadata PUBLIC ../include)

install(TARGETS MACCSMetadata DESTINATION lib)

install(TARGETS otbapp_ProductFormatter DESTINATION lib/otb/applications/)
