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

#ifndef RESAMPLE_AT_S2_RES_H
#define RESAMPLE_AT_S2_RES_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"
//Transform
#include "itkComposeImageFilter.h"
#include "itkScalableAffineTransform.h"
#include "ResamplingBandExtractor.h"
#include "MetadataHelperFactory.h"

#include "BandsCfgMappingParser.h"
#include "GenericRSImageResampler.h"

class ResampleAtS2Res
{
public:
    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef itk::VectorImage<PixelType, 2>             ImageType2;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;
    typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                         ImageType >                   ListConcatenerFilterType;

    typedef itk::ScalableAffineTransform<double, ImageType::ImageDimension> ScalableTransformType;
    typedef ScalableTransformType::OutputVectorType          OutputVectorType;


public:
    ResampleAtS2Res();
    void Init(const std::string &xml, const std::string &strMaskFileName,
              const std::string &bandsMappingFile, int nRes, const std::string &masterInfo,
              const std::string &primaryMissionXml);
    void DoExecute();

    ImageType::Pointer GetResampledMainImg();
    InternalImageType::Pointer GetResampledCloudMaskImg();
    InternalImageType::Pointer GetResampledWaterMaskImg();
    InternalImageType::Pointer GetResampledSnowMaskImg();
    InternalImageType::Pointer GetResampledAotImg();

    const char * GetNameOfClass() { return "ResampleAtS2Res"; }

private:
    ImageReaderType::Pointer getReader(const std::string& filePath);
    bool ExtractResampledMasksImages();
    void ExtractResampledAotImage();
    void CreateMasterInfoFile();
    void ExtractPrimaryMissionInfos();
    ImageType::Pointer ResampleImage(ImageType::Pointer img, Interpolator_Type interpolator, int &nCurRes);

private:
    std::string m_strXml;
    //if available, a primary mission could be provided in order
    //to get the origin, dimensions and projection
    std::string m_strPrimaryMissionXml;
    std::string m_strMaskFileName;

    ListConcatenerFilterType::Pointer     m_Concatener;

    ImageReaderListType::Pointer          m_ImageReaderList;
    InternalImageListType::Pointer        m_ImageList;

    InternalImageType::Pointer            m_ImageCloud;
    InternalImageType::Pointer            m_ImageWater;
    InternalImageType::Pointer            m_ImageSnow;
    InternalImageType::Pointer            m_ImageAot;

    int m_nRes;
    ImageType::Pointer m_inputImg;

    ResamplingBandExtractor<float> m_ResampledBandsExtractor;
    std::unique_ptr<MetadataHelper<float>> m_pMetadataHelper;
    std::unique_ptr<MetadataHelper<float>> m_pPrimaryMissionMetadataHelper;

    BandsCfgMappingParser m_bandsCfgMappingParser;
    std::string m_bandsMappingFile;
    std::string m_masterInfoFile;

    ImageType::PointType m_PrimaryMissionImgOrigin;
    std::string m_strPrMissionImgProjRef;
    double m_PrimaryMissionImgWidth;
    double m_PrimaryMissionImgHeight;
    bool m_bValidPrimaryImg;
    OutputVectorType m_scale;
    GenericRSImageResampler<ImageType, ImageType>  m_GenericRSImageResampler;
    ImageResampler<ImageType, ImageType>  m_ImageResampler;
    bool m_bUseGenericRSResampler;
};

#endif // RESAMPLE_AT_S2_RES_H
