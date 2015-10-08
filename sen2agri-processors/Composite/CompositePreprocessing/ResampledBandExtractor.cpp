#include "ResampledBandExtractor.h"

ResampledBandExtractor::ResampledBandExtractor()
{
    m_ResamplersList = ResampleFilterListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
}

ResampledBandExtractor::InternalImageType::Pointer ResampledBandExtractor::ExtractResampledBand(ImageType::Pointer img, int nChannel, int curRes,
                                              int nDesiredRes, bool bNearestNeighbourInterpolation)
{
    //Resample the cloud mask
    ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
    extractor->SetInput( img );
    extractor->SetChannel( nChannel );
    extractor->UpdateOutputInformation();
    m_ExtractorList->PushBack( extractor );

    return getResampledImage(curRes, nDesiredRes, extractor, bNearestNeighbourInterpolation);
}

ResampledBandExtractor::ResampleFilterType::Pointer ResampledBandExtractor::getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask) {
     ResampleFilterType::Pointer resampler = ResampleFilterType::New();
     resampler->SetInput(image);

     // Set the interpolator
     if(isMask) {
         NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
         resampler->SetInterpolator(interpolator);
     }
     else {
        LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
        resampler->SetInterpolator(interpolator);

        //BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
        //interpolator->SetRadius(2);
        //resampler->SetInterpolator(interpolator);
     }

     IdentityTransformType::Pointer transform = IdentityTransformType::New();

     resampler->SetOutputParametersFromImage( image );
     // Scale Transform
     OutputVectorType scale;
     scale[0] = 1.0 / ratio;
     scale[1] = 1.0 / ratio;

     // Evaluate spacing
     InternalImageType::SpacingType spacing = image->GetSpacing();
     InternalImageType::SpacingType OutputSpacing;
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
     ResampleFilterType::SizeType recomputedSize;
     recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
     recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

     resampler->SetOutputSize(recomputedSize);

     m_ResamplersList->PushBack(resampler);
     return resampler;
}

ResampledBandExtractor::InternalImageType::Pointer ResampledBandExtractor::getResampledImage(int nCurRes, int nDesiredRes,
                                             ExtractROIFilterType::Pointer extractor,
                                             bool bIsMask) {
    if((nDesiredRes <= 0) || nCurRes == nDesiredRes)
        return extractor->GetOutput();
    float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
    ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), fMultiplicationFactor, bIsMask);
    return resampler->GetOutput();
}

