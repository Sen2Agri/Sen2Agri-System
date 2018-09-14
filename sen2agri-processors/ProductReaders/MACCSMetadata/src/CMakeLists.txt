find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../Common/XmlUtils/include)

set(MACCSMetadata_HEADERS
    ../include/MACCSMetadata.hpp
    ../include/SPOT4Metadata.hpp
    ../include/MACCSMetadataReader.hpp
    ../include/MACCSMetadataWriter.hpp
    ../include/SPOT4MetadataReader.hpp
    ../include/ViewingAngles.hpp
    ../include/MetadataUtil.hpp)

set(MACCSMetadata_SOURCES
    MACCSMetadataReader.cpp
    MACCSMetadataWriter.cpp
    SPOT4MetadataReader.cpp
    ViewingAngles.cpp
    MetadataUtil.cpp)

add_library(MACCSMetadata SHARED ${MACCSMetadata_HEADERS} ${MACCSMetadata_SOURCES})
target_link_libraries(MACCSMetadata 
    Sen2AgriXmlUtils
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

target_include_directories(MACCSMetadata PUBLIC ../include)

install(TARGETS MACCSMetadata DESTINATION usr/lib/)
