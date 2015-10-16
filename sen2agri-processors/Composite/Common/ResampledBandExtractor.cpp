#include "ResampledBandExtractor.h"

ResampledBandExtractor::ResampledBandExtractor()
{
    m_ResamplersList = ResampleFilterListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
}

ResampledBandExtractor::InternalImageType::Pointer ResampledBandExtractor::ExtractResampledBand(const ImageType::Pointer img, int nChannel, int curRes,
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

ResampledBandExtractor::InternalImageType::Pointer ResampledBandExtractor::ExtractResampledBand2(const ImageType::Pointer img, int nChannel, int nWidth,
                                              int nHeight, bool bNearestNeighbourInterpolation)
{
    //Resample the cloud mask
    ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
    extractor->SetInput( img );
    extractor->SetChannel( nChannel );
    extractor->UpdateOutputInformation();
    m_ExtractorList->PushBack( extractor );

    return getResampledImage2(nWidth, nHeight, extractor, bNearestNeighbourInterpolation);
}

int ResampledBandExtractor::ExtractAllResampledBands(const ImageType::Pointer img,
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
        outList->PushBack(getResampledImage(curRes, nDesiredRes, extractor, bNearestNeighbourInterpolation));
    }

    return nbBands;
}

ResampledBandExtractor::ResampleFilterType::Pointer ResampledBandExtractor::getResampler(const InternalImageType::Pointer& image, const int wantedWidth,
                                                                                         const int wantedHeight, bool isMask)
{
    auto sz = image->GetLargestPossibleRegion().GetSize();
    OutputVectorType scale;
    scale[0] = (float)sz[0] / wantedWidth;
    scale[1] = (float)sz[1] / wantedHeight;
    ResampleFilterType::Pointer resampler = getResampler(image, scale, isMask);
    ResampleFilterType::SizeType recomputedSize;
    recomputedSize[0] = wantedWidth;
    recomputedSize[1] = wantedHeight;
    resampler->SetOutputSize(recomputedSize);
    return resampler;
}

ResampledBandExtractor::ResampleFilterType::Pointer ResampledBandExtractor::getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask) {
     // Scale Transform
     OutputVectorType scale;
     scale[0] = 1.0 / ratio;
     scale[1] = 1.0 / ratio;
     return getResampler(image, scale, isMask);
}

ResampledBandExtractor::ResampleFilterType::Pointer ResampledBandExtractor::getResampler(const InternalImageType::Pointer& image, const OutputVectorType& scale, bool isMask) {
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

ResampledBandExtractor::InternalImageType::Pointer ResampledBandExtractor::getResampledImage2(int nWidth, int nHeight,
                                             ExtractROIFilterType::Pointer extractor,
                                             bool bIsMask) {
    ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), nWidth, nHeight, bIsMask);
    return resampler->GetOutput();
}
