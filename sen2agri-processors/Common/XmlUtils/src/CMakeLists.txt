find_package(Boost REQUIRED COMPONENTS filesystem)

set(Sen2AgriXmlUtils_HEADERS
    ../include/FluentXML.hpp
    ../include/tinyxml_utils.hpp
    ../include/string_utils.hpp)

set(Sen2AgriXmlUtils_SOURCES
    FluentXML.cpp
    tinyxml_utils.cpp
    string_utils.cpp)

add_library(Sen2AgriXmlUtils SHARED ${Sen2AgriXmlUtils_HEADERS} ${Sen2AgriXmlUtils_SOURCES})
target_link_libraries(Sen2AgriXmlUtils
    "${Boost_LIBRARIES}"
    "${OTBCommon_LIBRARIES}"
    "${OTBTinyXML_LIBRARIES}"
    "${OTBITK_LIBRARIES}")

target_include_directories(Sen2AgriXmlUtils PUBLIC ../include)

install(TARGETS Sen2AgriXmlUtils DESTINATION usr/lib/)
