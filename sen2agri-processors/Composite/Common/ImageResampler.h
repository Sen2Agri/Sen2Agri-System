#ifndef IMAGE_RESAMPLER_H
#define IMAGE_RESAMPLER_H

#include "otbWrapperTypes.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"

template <class TImage>
class ImageResampler
{
public:
    typedef otb::StreamingResampleImageFilter<TImage, TImage, double>                           ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<TImage, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<TImage, double>                      LinearInterpolationType;
    typedef otb::BCOInterpolateImageFunction<TImage>                                 BCOInterpolationType;
    typedef itk::IdentityTransform<double, TImage::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, TImage::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

    typedef typename NearestNeighborInterpolationType::Pointer NearestNeighborInterpolationTypePtr;
    typedef typename LinearInterpolationType::Pointer LinearInterpolationTypePtr;
    typedef typename IdentityTransformType::Pointer IdentityTransformTypePtr;
    typedef typename TImage::Pointer ResamplerInputImgPtr;
    typedef typename TImage::PixelType ResamplerInputImgPixelType;
    typedef typename TImage::SpacingType ResamplerInputImgSpacingType;
    typedef typename otb::StreamingResampleImageFilter<TImage, TImage, double>::Pointer ResamplerPtr;
    typedef typename otb::StreamingResampleImageFilter<TImage, TImage, double>::SizeType ResamplerSizeType;
    typedef typename otb::ObjectList<ResampleFilterType>::Pointer   ResampleFilterListTypePtr;

public:
    const char * GetNameOfClass() { return "ImageResampler"; }

    ImageResampler()
    {
        m_ResamplersList = ResampleFilterListType::New();
    }


ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const int wantedWidth,
                                              const int wantedHeight, bool isMask=false)
    {
        auto sz = image->GetLargestPossibleRegion().GetSize();
        OutputVectorType scale;
        scale[0] = (float)sz[0] / wantedWidth;
        scale[1] = (float)sz[1] / wantedHeight;
        ResamplerPtr resampler = getResampler(image, scale, wantedWidth, wantedHeight, isMask);
        ResamplerSizeType recomputedSize;
        recomputedSize[0] = wantedWidth;
        recomputedSize[1] = wantedHeight;
        resampler->SetOutputSize(recomputedSize);
        return resampler;
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const float& ratio, bool isMask=false) {
         // Scale Transform
         OutputVectorType scale;
         scale[0] = 1.0 / ratio;
         scale[1] = 1.0 / ratio;
         return getResampler(image, scale, -1, -1, isMask);
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const OutputVectorType& scale,
                             int forcedWidth, int forcedHeight, bool isMask=false) {
         ResamplerPtr resampler = ResampleFilterType::New();
         resampler->SetInput(image);

         // Set the interpolator
         if(isMask) {
             NearestNeighborInterpolationTypePtr interpolator = NearestNeighborInterpolationType::New();
             resampler->SetInterpolator(interpolator);
         }
         else {
            LinearInterpolationTypePtr interpolator = LinearInterpolationType::New();
            resampler->SetInterpolator(interpolator);

            //BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
            //interpolator->SetRadius(2);
            //resampler->SetInterpolator(interpolator);
         }

         IdentityTransformTypePtr transform = IdentityTransformType::New();

         resampler->SetOutputParametersFromImage( image );

         // Evaluate spacing
         ResamplerInputImgSpacingType spacing = image->GetSpacing();
         ResamplerInputImgSpacingType OutputSpacing;
         OutputSpacing[0] = spacing[0] * scale[0];
         OutputSpacing[1] = spacing[1] * scale[1];

         resampler->SetOutputSpacing(OutputSpacing);

         otb::Wrapper::FloatVectorImageType::PointType origin = image->GetOrigin();
         otb::Wrapper::FloatVectorImageType::PointType outputOrigin;
         outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
         outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

         resampler->SetOutputOrigin(outputOrigin);

         resampler->SetTransform(transform);

         // Evaluate size
         ResamplerSizeType recomputedSize;
         if((forcedWidth != -1) && (forcedHeight != -1))
         {
             recomputedSize[0] = forcedWidth;
             recomputedSize[1] = forcedHeight;
         } else {
            recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
            recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];
         }

         resampler->SetOutputSize(recomputedSize);

         ResamplerInputImgPixelType defaultValue;
         itk::NumericTraits<ResamplerInputImgPixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
         if(!isMask) {
             defaultValue = NO_DATA_VALUE;
         }
         resampler->SetEdgePaddingValue(defaultValue);

    //     recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
    //     recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

    //     resampler->SetOutputSize(recomputedSize);

         m_ResamplersList->PushBack(resampler);
         return resampler;
    }
private:
    ResampleFilterListTypePtr             m_ResamplersList;
};

#endif // IMAGE_RESAMPLER_H

