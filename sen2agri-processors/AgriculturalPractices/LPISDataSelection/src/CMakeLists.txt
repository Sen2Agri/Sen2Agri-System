find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../Common/include)

otb_create_application(
  NAME           LPISDataSelection
  SOURCES        ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
                 ../../Common/include/DataSource.h ../../Common/include/DataSourceFactory.h
                 ../../Common/include/ShpDataSource.h ../../Common/include/PostgisDataSource.h
                 LPISDataSelection.cxx
                 CountryInfoBase.h CountryInfoBase.cpp
                 CountryInfoFactory.h
                 CzeCountryInfo.h CzeCountryInfo.cpp
                 EspCountryInfo.h EspCountryInfo.cpp
                 ItaCountryInfo.h ItaCountryInfo.cpp
                 LtuCountryInfo.h LtuCountryInfo.cpp
                 NlCountryInfo.h NlCountryInfo.cpp
                 RouCountryInfo.h RouCountryInfo.cpp
                 AttributeEntry.h
                 GSAAAttributesTablesReaderBase.h
                 GSAAAttributesTablesReaderFactory.cpp
                 GSAAAttributesTablesReaderFactory.h
                 GSAACsvAttributesTablesReader.cpp
                 GSAACsvAttributesTablesReader.h
                 GSAAShpAttributesTablesReader.cpp
                 GSAAShpAttributesTablesReader.h

  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_LPISDataSelection DESTINATION usr/lib/otb/applications/)

