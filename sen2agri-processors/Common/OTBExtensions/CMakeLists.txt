set(OTBExtensions_HEADERS otbConcatenateVectorImagesFilter.h otbConcatenateVectorImagesFilter.txx
                          otbGridResampleImageFilter.h otbGridResampleImageFilter.txx)

add_custom_target(OTBExtensions SOURCES ${OTBExtensions_HEADERS})
