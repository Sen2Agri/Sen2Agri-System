#include "ResamplingBandExtractor.h"


ResamplingBandExtractor::ResamplingBandExtractor()
{
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
}

ResamplingBandExtractor::InternalImageType::Pointer ResamplingBandExtractor::ExtractResampledBand(
        const std::string &filePath, int nChannel, int nCurRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight,
        bool bNearestNeighbourInterpolation)
{
    // get a reader from the file path
    ImageReaderType::Pointer reader = ImageReaderType::New();
    // add it to the list and return
    m_ImageReaderList->PushBack(reader);
    // set the file name
    reader->SetFileName(filePath);
    reader->UpdateOutputInformation();
    if(nDesiredRes > 0 && nCurRes == -1) {
        nCurRes = reader->GetOutput()->GetSpacing()[0];
    }
    return ExtractResampledBand(reader->GetOutput(), nChannel, nCurRes,
                                nDesiredRes, nForcedOutWidth, nForcedOutHeight,
                                bNearestNeighbourInterpolation);
}

int ResamplingBandExtractor::ExtractAllResampledBands(const std::string &filePath,
                                                      otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                                      int nCurRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight,
                                                      bool bNearestNeighbourInterpolation)
{
    // get a reader from the file path
    ImageReaderType::Pointer reader = ImageReaderType::New();
    // add it to the list and return
    m_ImageReaderList->PushBack(reader);
    // set the file name
    reader->SetFileName(filePath);
    reader->UpdateOutputInformation();
    if(nDesiredRes > 0 && nCurRes == -1) {
        nCurRes = reader->GetOutput()->GetSpacing()[0];
    }
    return ExtractAllResampledBands(reader->GetOutput(), outList, nCurRes, nDesiredRes, nForcedOutWidth,
                             nForcedOutHeight, bNearestNeighbourInterpolation);
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
                                              int curRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight, bool bNearestNeighbourInterpolation)
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
        outList->PushBack(getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, bNearestNeighbourInterpolation));
    }

    return nbBands;
}

ResamplingBandExtractor::InternalImageType::Pointer ResamplingBandExtractor::getResampledImage(int nCurRes, int nDesiredRes,
                                             int forcedWidth, int forcedHeight, ExtractROIFilterType::Pointer extractor,
                                             bool bIsMask) {
    // if the resolutions are identical AND desired dimensions are not set
    if(nDesiredRes <= 0) {
        return extractor->GetOutput();
    }
    if(nCurRes == nDesiredRes) {
        if((forcedWidth == -1) || (forcedHeight == -1)) {
            return extractor->GetOutput();
        }
        // if we have the same resolution and the same desired dimensions as the input image
        extractor->GetOutput()->UpdateOutputInformation();
        auto sz = extractor->GetOutput()->GetLargestPossibleRegion().GetSize();
        // no need to do any resampling if res and dimensions are the same
        if((sz[0] == (unsigned int)forcedWidth) && (sz[1] == (unsigned int)forcedHeight)) {
            return extractor->GetOutput();
        }
    }

    OutputVectorType scale;
    scale[0] = ((float)nDesiredRes) / nCurRes;
    scale[1] = ((float)nDesiredRes) / nCurRes;

    ImageResampler<InternalImageType>::ResamplerPtr resampler = m_ImageResampler.getResampler(
                extractor->GetOutput(), scale, forcedWidth, forcedHeight, bIsMask);
    return resampler->GetOutput();
}
