otb_create_application(
  NAME           CropMaskTrainImagesClassifier
  SOURCES        otbTrainImagesClassifier.cxx otbTrainImagesClassifier.h
                 ../Filters/CropMaskPreprocessing.h
                 ../Filters/CropMaskNDVIPreprocessing.h
                 ../Filters/CropMaskFeaturesSupervised.hxx

                 ../Filters/ComputeNDVIFilter.h
                 ../Filters/TemporalMerging.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../Common/OTBExtensions)

add_dependencies(otbapp_BandsExtractor OTBExtensions)

install(TARGETS otbapp_CropMaskTrainImagesClassifier DESTINATION usr/lib/otb/applications/)
