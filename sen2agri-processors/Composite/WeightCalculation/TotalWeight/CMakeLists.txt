otb_create_application(
  NAME           TotalWeight
  SOURCES        totalweightcomputation.cpp totalweightapp.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

