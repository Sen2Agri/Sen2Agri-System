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

#ifndef RESAMPLING_BAND_EXTRACTOR_H
#define RESAMPLING_BAND_EXTRACTOR_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageListToVectorImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"

#include "ImageResampler.h"

template <typename PixelType>
class ResamplingBandExtractor
{
public:
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;

    typedef otb::MultiToMonoChannelExtractROI<PixelType,
                                              PixelType>     ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                               ExtractROIFilterListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

public:
    ResamplingBandExtractor()
    {
        m_ExtractorList = ExtractROIFilterListType::New();
        m_ImageReaderList = ImageReaderListType::New();
    }

    typename InternalImageType::Pointer ExtractResampledBand(const std::string &filePath, int nChannel,
                                                  Interpolator_Type interpolator,
                                                  int nCurRes = -1, int nDesiredRes=-1, int
                                                  nForcedOutWidth=-1, int nForcedOutHeight=-1)
    {
        // get a reader from the file path
        typename ImageReaderType::Pointer reader = ImageReaderType::New();
        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        // set the file name
        reader->SetFileName(filePath);
        reader->UpdateOutputInformation();
        if(nDesiredRes > 0 && nCurRes == -1) {
            nCurRes = reader->GetOutput()->GetSpacing()[0];
        }
        return ExtractImgResampledBand(reader->GetOutput(), nChannel,
                                    interpolator, nCurRes,nDesiredRes,
                                    nForcedOutWidth, nForcedOutHeight);
    }

    int ExtractAllResampledBands(const std::string &filePath, typename otb::ImageList<otb::Image<PixelType>>::Pointer &outList,
                                 Interpolator_Type interpolator,
                                 int nCurRes = -1, int nDesiredRes=-1,
                                 int nForcedOutWidth=-1, int nForcedOutHeight=-1)
    {
        // get a reader from the file path
        typename ImageReaderType::Pointer reader = ImageReaderType::New();
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

    typename InternalImageType::Pointer ExtractImgResampledBand(const typename ImageType::Pointer img, int nChannel,
                                                  Interpolator_Type interpolator,
                                                  int curRes=-1, int nDesiredRes=-1,
                                                  int nForcedOutWidth=-1, int nForcedOutHeight=-1)
    {
        //Resample the cloud mask
        typename ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
        extractor->SetInput( img );
        extractor->SetChannel( nChannel );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        return getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, interpolator);
    }

    int ExtractAllResampledBands(const typename ImageType::Pointer img, typename otb::ImageList<otb::Image<PixelType>>::Pointer &outList,
                                 Interpolator_Type interpolator,
                                 int curRes=-1, int nDesiredRes=-1,
                                 int nForcedOutWidth=-1, int nForcedOutHeight=-1)
    {
        int nbBands = img->GetNumberOfComponentsPerPixel();
        for(int j=0; j < nbBands; j++)
        {

            //Resample the cloud mask
            typename ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( img );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            // TODO: see if this function should receive instead the forced size of a reference image, if possible
            outList->PushBack(getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, interpolator));
        }

        return nbBands;
    }

    std::vector<int> ExtractImageBands(const typename ImageType::Pointer img, typename otb::ImageList<otb::Image<PixelType>>::Pointer &outList,
                                 const std::vector<int> &bandsIndexes, Interpolator_Type interpolator,
                                 int curRes=-1, int nDesiredRes=-1,
                                 int nForcedOutWidth=-1, int nForcedOutHeight=-1)
    {
        int nbBands = img->GetNumberOfComponentsPerPixel();
        std::vector<int> relBandsIndexes(bandsIndexes.size());
        std::fill(relBandsIndexes.begin(), relBandsIndexes.end(), -1);
        int curBandIdx = outList->Size();
        for(int j=0; j < bandsIndexes.size(); j++)
        {
            if (bandsIndexes[j] < nbBands) {
                //Resample the cloud mask
                typename ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
                extractor->SetInput( img );
                extractor->SetChannel( bandsIndexes[j] + 1 );
                extractor->UpdateOutputInformation();
                m_ExtractorList->PushBack( extractor );
                outList->PushBack(getResampledImage(curRes, nDesiredRes, nForcedOutWidth, nForcedOutHeight, extractor, interpolator));
                relBandsIndexes[j] = curBandIdx;
                curBandIdx++;
            }
        }

        return relBandsIndexes;
    }

    typename InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes, int forcedWidth, int forcedHeight,
                                                 typename ExtractROIFilterType::Pointer extractor,
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

        typename ImageResampler<InternalImageType, InternalImageType>::ResamplerPtr resampler = m_ImageResampler.getResampler(
                    extractor->GetOutput(), scale, forcedWidth, forcedHeight, interpolator);
        return resampler->GetOutput();
    }

    const char * GetNameOfClass() { return "ResamplingBandExtractor"; }

private:
    typename ExtractROIFilterListType::Pointer     m_ExtractorList;
    typename ImageReaderListType::Pointer          m_ImageReaderList;
    ImageResampler<InternalImageType, InternalImageType>     m_ImageResampler;

};

#endif // RESAMPLING_BAND_EXTRACTOR_H



