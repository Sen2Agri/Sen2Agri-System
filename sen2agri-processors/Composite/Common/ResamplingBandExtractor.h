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

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"

#include "ImageResampler.h"

class ResamplingBandExtractor1
{
public:
    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;

    typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                              InternalImageType::PixelType>     ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                               ExtractROIFilterListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

public:
    ResamplingBandExtractor1();
    InternalImageType::Pointer ExtractResampledBand(const std::string &fileName, int nChannel,
                                                  Interpolator_Type interpolator,
                                                  int nCurRes = -1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1);

    int ExtractAllResampledBands(const std::string &fileName, otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                 Interpolator_Type interpolator,
                                 int nCurRes = -1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1);

    InternalImageType::Pointer ExtractImgResampledBand(const ImageType::Pointer img, int nChannel,
                                                  Interpolator_Type interpolator, int curRes=-1,
                                                  int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1);

    int ExtractAllResampledBands(const ImageType::Pointer img, otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                Interpolator_Type interpolator,
                                int curRes=-1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1);

    const char * GetNameOfClass() { return "ResamplingBandExtractor1"; }

private:
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes, int forcedWidth, int forcedHeight,
                                                 ExtractROIFilterType::Pointer extractor,
                                                 Interpolator_Type interpolator);
private:
    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    ImageResampler<InternalImageType, InternalImageType>     m_ImageResampler;
};

#endif // RESAMPLING_BAND_EXTRACTOR_H



