find_package(Boost REQUIRED COMPONENTS regex)

include_directories(../../Common/include)

otb_create_application(
  NAME           LPISDataSelection
  SOURCES        ../../Common/include/CommonFunctions.h
                 ../../Common/include/CommonDefs.h
                 ../../Common/include/DataSource.h
                 ../../Common/include/DataSourceFactory.h
                 ../../Common/include/ShpDataSource.h
                 ../../Common/include/PostgisDataSource.h
                 ../../Common/include/AttributeEntry.h
                 ../../Common/include/GSAAAttributesTablesReaderBase.h
                 ../../Common/src/GSAAAttributesTablesReaderFactory.cpp
                 ../../Common/include/GSAAAttributesTablesReaderFactory.h
                 ../../Common/src/GSAACsvAttributesTablesReader.cpp
                 ../../Common/include/GSAACsvAttributesTablesReader.h
                 ../../Common/src/GSAAShpAttributesTablesReader.cpp
                 ../../Common/include/GSAAShpAttributesTablesReader.h
                 ../../Common/src/DeclarationsInfoBase.cpp
                 ../../Common/include/DeclarationsInfoBase.h
                 LPISDataSelection.cxx
                 CountryInfoBase.h CountryInfoBase.cpp
                 CountryInfoFactory.h
                 CzeCountryInfo.h CzeCountryInfo.cpp
                 EspCountryInfo.h EspCountryInfo.cpp
                 ItaCountryInfo.h ItaCountryInfo.cpp
                 LtuCountryInfo.h LtuCountryInfo.cpp
                 NlCountryInfo.h NlCountryInfo.cpp
                 RouCountryInfo.h RouCountryInfo.cpp
                 FraCountryInfo.h FraCountryInfo.cpp
                 DefaultCountryInfo.h DefaultCountryInfo.cpp
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata ${Boost_LIBRARIES})

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_LPISDataSelection DESTINATION usr/lib/otb/applications/)

