otb_create_application(
    NAME           SpectralFeaturesExtraction
    SOURCES        SpectralFeaturesExtraction.cpp
                   ../../Common/Filters/DataSmoothingFilter.h
                   ../../Common/Filters/CropMaskSpectralFeaturesFilter.h
    LINK_LIBRARIES ${OTB_LIBRARIES}
                   Sen2AgriProductReaders
                   TimeSeriesReader)

include_directories(../../Common/OTBExtensions)
include_directories(../../Common/Filters)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

add_dependencies(otbapp_SpectralFeaturesExtraction OTBExtensions)

install(TARGETS otbapp_SpectralFeaturesExtraction DESTINATION usr/lib/otb/applications/)
