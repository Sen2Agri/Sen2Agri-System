#include "ResamplingBandExtractor.h"

ResamplingBandExtractor::ResamplingBandExtractor()
{
    m_ResamplersList = ResampleFilterListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
}

ResamplingBandExtractor::InternalImageType::Pointer ResamplingBandExtractor::ExtractResampledBand(const ImageType::Pointer img, int nChannel, int curRes,
                                              int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight, bool bNearestNeighbourInterpolation)
{
    //Resample the cloud mask
    ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
    extractor->SetInput( img );
    extractor->SetChannel( nChannel );
    extractor->UpdateOutputInformation();
    m_ExtractorList->PushBack( extractor );

    return getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, bNearestNeighbourInterpolation);
}

int ResamplingBandExtractor::ExtractAllResampledBands(const ImageType::Pointer img,
                                              otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                              int curRes, int nDesiredRes, bool bNearestNeighbourInterpolation)
{
    int nbBands = img->GetNumberOfComponentsPerPixel();
    for(int j=0; j < nbBands; j++)
    {

        //Resample the cloud mask
        ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
        extractor->SetInput( img );
        extractor->SetChannel( j+1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        // TODO: see if this function should receive instead the forced size of a reference image, if possible
        outList->PushBack(getResampledImage(curRes, nDesiredRes, -1, -1, extractor, bNearestNeighbourInterpolation));
    }

    return nbBands;
}

ResamplingBandExtractor::ResampleFilterType::Pointer ResamplingBandExtractor::getResampler(const InternalImageType::Pointer& image, const int wantedWidth,
                                                                                         const int wantedHeight, bool isMask)
{
    auto sz = image->GetLargestPossibleRegion().GetSize();
    OutputVectorType scale;
    scale[0] = (float)sz[0] / wantedWidth;
    scale[1] = (float)sz[1] / wantedHeight;
    ResampleFilterType::Pointer resampler = getResampler(image, scale, wantedWidth, wantedHeight, isMask);
    ResampleFilterType::SizeType recomputedSize;
    recomputedSize[0] = wantedWidth;
    recomputedSize[1] = wantedHeight;
    resampler->SetOutputSize(recomputedSize);
    return resampler;
}

ResamplingBandExtractor::ResampleFilterType::Pointer ResamplingBandExtractor::getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask) {
     // Scale Transform
     OutputVectorType scale;
     scale[0] = 1.0 / ratio;
     scale[1] = 1.0 / ratio;
     return getResampler(image, scale, -1, -1, isMask);
}

ResamplingBandExtractor::ResampleFilterType::Pointer ResamplingBandExtractor::getResampler(const InternalImageType::Pointer& image, const OutputVectorType& scale,
                                                                                         int forcedWidth, int forcedHeight, bool isMask) {
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
     if((forcedWidth != -1) && (forcedHeight != -1))
     {
         recomputedSize[0] = forcedWidth;
         recomputedSize[1] = forcedHeight;
     } else {
        recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
        recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];
     }

     resampler->SetOutputSize(recomputedSize);

     InternalImageType::PixelType defaultValue;
     itk::NumericTraits<InternalImageType::PixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
     resampler->SetEdgePaddingValue(defaultValue);

//     recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
//     recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

//     resampler->SetOutputSize(recomputedSize);

     m_ResamplersList->PushBack(resampler);
     return resampler;
}

ResamplingBandExtractor::InternalImageType::Pointer ResamplingBandExtractor::getResampledImage(int nCurRes, int nDesiredRes,
                                             int forcedWidth, int forcedHeight, ExtractROIFilterType::Pointer extractor,
                                             bool bIsMask) {
    if((nDesiredRes <= 0) || nCurRes == nDesiredRes)
        return extractor->GetOutput();

    OutputVectorType scale;
    scale[0] = ((float)nDesiredRes) / nCurRes;
    scale[1] = ((float)nDesiredRes) / nCurRes;

    ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), scale, forcedWidth, forcedHeight, bIsMask);
    return resampler->GetOutput();
}

