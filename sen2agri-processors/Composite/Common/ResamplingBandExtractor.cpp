/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#include "ResamplingBandExtractor.h"


ResamplingBandExtractor1::ResamplingBandExtractor1()
{
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
}

ResamplingBandExtractor1::InternalImageType::Pointer ResamplingBandExtractor1::ExtractResampledBand(const std::string &filePath, int nChannel,
        Interpolator_Type interpolator, int nCurRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight)
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
    return ExtractImgResampledBand(reader->GetOutput(), nChannel, interpolator, nCurRes,
                                nDesiredRes, nForcedOutWidth, nForcedOutHeight);
}

int ResamplingBandExtractor1::ExtractAllResampledBands(const std::string &filePath,
                                                      otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                                      Interpolator_Type interpolator,
                                                      int nCurRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight)
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
    return ExtractAllResampledBands(reader->GetOutput(), outList, interpolator, nCurRes, nDesiredRes, nForcedOutWidth,
                             nForcedOutHeight);
}

ResamplingBandExtractor1::InternalImageType::Pointer ResamplingBandExtractor1::ExtractImgResampledBand(const ImageType::Pointer img,
                                              int nChannel, Interpolator_Type interpolator, int curRes,
                                              int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight)
{
    //Resample the cloud mask
    ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
    extractor->SetInput( img );
    extractor->SetChannel( nChannel );
    extractor->UpdateOutputInformation();
    m_ExtractorList->PushBack( extractor );

    return getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, interpolator);
}

int ResamplingBandExtractor1::ExtractAllResampledBands(const ImageType::Pointer img,
                                              otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList, Interpolator_Type interpolator,
                                              int curRes, int nDesiredRes, int nForcedOutWidth, int nForcedOutHeight)
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
        outList->PushBack(getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, interpolator));
    }

    return nbBands;
}

ResamplingBandExtractor1::InternalImageType::Pointer ResamplingBandExtractor1::getResampledImage(int nCurRes, int nDesiredRes,
                                             int forcedWidth, int forcedHeight, ExtractROIFilterType::Pointer extractor,
                                             Interpolator_Type interpolator) {
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

    ImageResampler<InternalImageType, InternalImageType>::ResamplerPtr resampler = m_ImageResampler.getResampler(
                extractor->GetOutput(), scale, forcedWidth, forcedHeight, interpolator);
    return resampler->GetOutput();
}
