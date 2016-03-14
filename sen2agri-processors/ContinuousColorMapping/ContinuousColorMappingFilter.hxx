#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkUnaryFunctorImageFilter.h>

#include "ContinuousColorMappingFunctor.hxx"


typedef itk::UnaryFunctorImageFilter<otb::Wrapper::FloatImageType,
                                     otb::Wrapper::UInt8RGBImageType,
                                     ContinuousColorMappingFunctor>
        ContinuousColorMappingFilter;
