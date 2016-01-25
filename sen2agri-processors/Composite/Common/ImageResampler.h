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

typedef enum
{
  Interpolator_NNeighbor,
  Interpolator_Linear,
  Interpolator_BCO
} Interpolator_Type;

template <class TInput, class TOutput>
class ImageResampler
{
public:

    typedef otb::StreamingResampleImageFilter<TInput, TOutput, double>                           ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<TOutput, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<TOutput, double>                      LinearInterpolationType;
    typedef otb::BCOInterpolateImageFunction<TOutput>                                 BCOInterpolationType;
    typedef itk::IdentityTransform<double, TOutput::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, TOutput::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

    typedef typename NearestNeighborInterpolationType::Pointer NearestNeighborInterpolationTypePtr;
    typedef typename LinearInterpolationType::Pointer LinearInterpolationTypePtr;
    typedef typename IdentityTransformType::Pointer IdentityTransformTypePtr;
    typedef typename TInput::Pointer ResamplerInputImgPtr;
    typedef typename TInput::PixelType ResamplerInputImgPixelType;
    typedef typename TInput::SpacingType ResamplerInputImgSpacingType;
    typedef typename ResampleFilterType::Pointer ResamplerPtr;
    typedef typename ResampleFilterType::SizeType ResamplerSizeType;
    typedef typename otb::ObjectList<ResampleFilterType>::Pointer   ResampleFilterListTypePtr;

public:
    const char * GetNameOfClass() { return "ImageResampler"; }

    ImageResampler()
    {
        m_ResamplersList = ResampleFilterListType::New();
        m_BCORadius = 2;
        m_fBCOAlpha = -0.5;
    }

    void SetBicubicInterpolatorParameters(int BCORadius, float BCOAlpha = -0.5) {
        m_BCORadius = BCORadius;
        m_fBCOAlpha = BCOAlpha;
    }


ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const int wantedWidth,
                          const int wantedHeight, Interpolator_Type interpolator=Interpolator_Linear)
    {
        auto sz = image->GetLargestPossibleRegion().GetSize();
        OutputVectorType scale;
        scale[0] = (float)sz[0] / wantedWidth;
        scale[1] = (float)sz[1] / wantedHeight;
        ResamplerPtr resampler = getResampler(image, scale, wantedWidth, wantedHeight, interpolator);
        ResamplerSizeType recomputedSize;
        recomputedSize[0] = wantedWidth;
        recomputedSize[1] = wantedHeight;
        resampler->SetOutputSize(recomputedSize);
        return resampler;
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const float& ratio,
                              Interpolator_Type interpolator=Interpolator_Linear) {
         // Scale Transform
         OutputVectorType scale;
         scale[0] = 1.0 / ratio;
         scale[1] = 1.0 / ratio;
         return getResampler(image, scale, -1, -1, interpolator);
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const OutputVectorType& scale,
                             int forcedWidth, int forcedHeight, Interpolator_Type interpolator=Interpolator_Linear) {
         ResamplerPtr resampler = ResampleFilterType::New();
         resampler->SetInput(image);

         // Set the interpolator
         switch ( interpolator )
         {
             case Interpolator_Linear:
             {
                 typename LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
                 resampler->SetInterpolator(interpolator);
             }
             break;
             case Interpolator_NNeighbor:
             {
                 typename NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
                 resampler->SetInterpolator(interpolator);
             }
             break;
             case Interpolator_BCO:
             {
                 typename BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
                 interpolator->SetRadius(m_BCORadius);
                 interpolator->SetAlpha(m_fBCOAlpha);
                 resampler->SetInterpolator(interpolator);
             }
             break;
         }

         IdentityTransformTypePtr transform = IdentityTransformType::New();

         resampler->SetOutputParametersFromImage( image );

         // Evaluate spacing
         ResamplerInputImgSpacingType spacing = image->GetSpacing();
         ResamplerInputImgSpacingType OutputSpacing;
         OutputSpacing[0] = std::round(spacing[0] * scale[0]);
         OutputSpacing[1] = std::round(spacing[1] * scale[1]);

         resampler->SetOutputSpacing(OutputSpacing);

         typename TOutput::PointType origin = image->GetOrigin();
         typename TOutput::PointType outputOrigin;
         outputOrigin[0] = std::round(origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0));
         outputOrigin[1] = std::round(origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0));

         resampler->SetOutputOrigin(outputOrigin);
         resampler->SetTransform(transform);

         // Evaluate size
         ResamplerSizeType recomputedSize;
         if((forcedWidth != -1) && (forcedHeight != -1))
         {
             recomputedSize[0] = forcedWidth;
             recomputedSize[1] = forcedHeight;
         } else {
//            recomputedSize[0] = vcl_ceil(image->GetLargestPossibleRegion().GetSize()[0] / scale[0]);
//            recomputedSize[1] = vcl_ceil(image->GetLargestPossibleRegion().GetSize()[1] / scale[1]);
            recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
            recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

         }

         resampler->SetOutputSize(recomputedSize);

         ResamplerInputImgPixelType defaultValue;
         itk::NumericTraits<ResamplerInputImgPixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
         if(interpolator != Interpolator_NNeighbor) {
             defaultValue = NO_DATA_VALUE;
         }
         resampler->SetEdgePaddingValue(defaultValue);

         m_ResamplersList->PushBack(resampler);
         return resampler;
    }
private:
    ResampleFilterListTypePtr             m_ResamplersList;
    int     m_BCORadius;
    float   m_fBCOAlpha;
};

#endif // IMAGE_RESAMPLER_H

