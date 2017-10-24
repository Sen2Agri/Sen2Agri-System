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
 
#include "ResampleAtS2Res_2.h"


ResampleAtS2Res2::ResampleAtS2Res2()
{
    m_PrimaryMissionImgOrigin[0] = -1;
    m_PrimaryMissionImgOrigin[1] = -1;
    m_bValidPrimaryImg = false;
    m_bUseGenericRSResampler = false;
}

void ResampleAtS2Res2::Init(const std::string &xml, const std::string &strMaskFileName,
                            const std::string &bandsMappingFile, int nRes, const std::string &masterInfo,
                            const std::string &primaryMissionXml)
{
    m_strXml = xml;
    m_strMaskFileName = strMaskFileName;
    m_nRes = nRes;

    m_Concatener = ListConcatenerFilterType::New();
    m_ImageList = InternalImageListType::New();
    m_ImageReaderList = ImageReaderListType::New();

    m_bandsMappingFile = bandsMappingFile;
    m_masterInfoFile = masterInfo;
    m_strPrimaryMissionXml = primaryMissionXml;
}

void ResampleAtS2Res2::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    m_pMetadataHelper = ((m_nRes <= 0) ? factory->GetMetadataHelper(m_strXml):
                                        factory->GetMetadataHelper(m_strXml, m_nRes));

    std::string imageFile = m_pMetadataHelper->GetImageFileName();
    m_inputImgReader = getReader(imageFile);
    m_inputImgReader->UpdateOutputInformation();
    ImageType::Pointer img = m_inputImgReader->GetOutput();
    int curRes = img->GetSpacing()[0];
    if(m_nRes <= 0) {
        m_nRes = curRes;
    }

    m_bandsCfgMappingParser.ParseFile(m_bandsMappingFile);

    // extract the primary mission infos (origin and projection) if it is the case
    ExtractPrimaryMissionInfos();

    // resample the image if it is the case
    img = ResampleImage(img, Interpolator_Linear, curRes);

    std::string missionName = m_pMetadataHelper->GetMissionName();
    BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
    const std::vector<int> &vectIdxs = bandsMappingCfg.GetAbsoluteBandIndexes(m_nRes, missionName);
    for(unsigned int i = 0; i<vectIdxs.size(); i++) {
        int nRelBandIdx = m_pMetadataHelper->GetRelativeBandIndex(vectIdxs[i]);
        m_ImageList->PushBack(m_ResampledBandsExtractor.ExtractImgResampledBand(img, nRelBandIdx, Interpolator_Linear, curRes, m_nRes));
    }

    // Extract (and resample, if needed) the cloud, water and snow masks
    ExtractResampledMasksImages();

    // Extract (and resample, if needed) the AOT
    ExtractResampledAotImage();

    m_Concatener->SetInput( m_ImageList );

    CreateMasterInfoFile();
}

ResampleAtS2Res2::ImageType::Pointer ResampleAtS2Res2::GetResampledMainImg() {
    return m_Concatener->GetOutput();
}

ResampleAtS2Res2::InternalImageType::Pointer ResampleAtS2Res2::GetResampledCloudMaskImg() {
    return m_ImageCloud;
}

ResampleAtS2Res2::InternalImageType::Pointer ResampleAtS2Res2::GetResampledWaterMaskImg() {
    return m_ImageWater;
}

ResampleAtS2Res2::InternalImageType::Pointer ResampleAtS2Res2::GetResampledSnowMaskImg() {
    return m_ImageSnow;
}

ResampleAtS2Res2::InternalImageType::Pointer ResampleAtS2Res2::GetResampledAotImg() {
    return m_ImageAot;
}

// get a reader from the file path
ResampleAtS2Res2::ImageReaderType::Pointer ResampleAtS2Res2::getReader(const std::string& filePath) {
    ImageReaderType::Pointer reader = ImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath);

    // add it to the list and return
    m_ImageReaderList->PushBack(reader);
    return reader;
}

void ResampleAtS2Res2::ExtractResampledAotImage()
{
    // resample the AOT
    std::string aotFileName = m_pMetadataHelper->GetAotImageFileName();
    int aotBandIdx = m_pMetadataHelper->GetAotBandIndex();
    ImageReaderType::Pointer aotImageReader = getReader(aotFileName);
    ImageType::Pointer aotImage = aotImageReader->GetOutput();
    aotImage->UpdateOutputInformation();
    int curRes = aotImage->GetSpacing()[0];
    //auto sz = m_inputImgReader->GetOutput()->GetLargestPossibleRegion().GetSize();
    //m_ImageAot = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, aotBandIdx, curRes, m_nRes, sz[0], sz[1], true);
    // TODO: Here we should take into account also the sz[0] and sz[1] but unfortunatelly,
    //       the input image is likely to be also resampled so we should receive them from outside

    // resample the image if it is the case
    aotImage = ResampleImage(aotImage, Interpolator_Linear, curRes);

    m_ImageAot = m_ResampledBandsExtractor.ExtractImgResampledBand(aotImage, aotBandIdx, Interpolator_Linear, curRes, m_nRes);

}

bool ResampleAtS2Res2::ExtractResampledMasksImages()
{
    if(m_strMaskFileName == "") {
        itkExceptionMacro("The mask file was not provided, do set 'msk' flag ");
        return false;
    }
    ImageReaderType::Pointer masksReader = getReader(m_strMaskFileName);
    ImageType::Pointer img = masksReader->GetOutput();
    img->UpdateOutputInformation();
    int curRes = img->GetSpacing()[0];

    // resample the image if it is the case
    img = ResampleImage(img, Interpolator_NNeighbor, curRes);

    //Extract and Resample the cloud mask
    m_ImageCloud = m_ResampledBandsExtractor.ExtractImgResampledBand(img, 1, Interpolator_NNeighbor, curRes, m_nRes);
    //Resample the water mask
    m_ImageWater = m_ResampledBandsExtractor.ExtractImgResampledBand(img, 2, Interpolator_NNeighbor, curRes, m_nRes);
    //Resample the snow mask
    m_ImageSnow = m_ResampledBandsExtractor.ExtractImgResampledBand(img, 3, Interpolator_NNeighbor, curRes, m_nRes);

    return true;
}

void ResampleAtS2Res2::CreateMasterInfoFile() {
    if(m_masterInfoFile != "") {
        const BandsMappingConfig &bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
        std::string curMissionName = m_pMetadataHelper->GetMissionName();
        if(bandsMappingCfg.IsMasterMission(curMissionName)) {
            try {
              std::ofstream outFile(m_masterInfoFile);
              outFile << curMissionName << std::endl;
            } catch(...) {
              itkGenericExceptionMacro(<< "Could not create file " << m_masterInfoFile);
            }
        }
    }
}

void ResampleAtS2Res2::ExtractPrimaryMissionInfos() {
    if(m_strPrimaryMissionXml.size() > 0)
    {
        auto factory = MetadataHelperFactory::New();
        m_pPrimaryMissionMetadataHelper = ((m_nRes <= 0) ? factory->GetMetadataHelper(m_strPrimaryMissionXml):
                                            factory->GetMetadataHelper(m_strPrimaryMissionXml, m_nRes));
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
        std::string missionName = m_pPrimaryMissionMetadataHelper->GetMissionName();
        std::string curMissionName = m_pMetadataHelper->GetMissionName();
        // only if the primary mission is the primary mission and is a different mission than the current one
        if((missionName != curMissionName) && bandsMappingCfg.IsMasterMission(missionName))
        {
            std::string imageFile = m_pPrimaryMissionMetadataHelper->GetImageFileName();
            ImageReaderType::Pointer reader = ImageReaderType::New();
            // set the file name
            reader->SetFileName(imageFile);
            reader->UpdateOutputInformation();
            ImageType::Pointer img = reader->GetOutput();
            int primaryImgRes = img->GetSpacing()[0];
            if(primaryImgRes != m_nRes)
            {
                m_nRes = primaryImgRes;
            }

            ImageType::Pointer oldImg = m_inputImgReader->GetOutput();
            const std::string &oldImgProjRef = oldImg->GetProjectionRef();
            int oldImgRes = oldImg->GetSpacing()[0];

            // get the origin and the projection of the main mission
            const float scale = (float)m_nRes / oldImgRes;
            m_scale[0] = scale;
            m_scale[1] = scale;

            m_PrimaryMissionImgWidth = img->GetLargestPossibleRegion().GetSize()[0];
            m_PrimaryMissionImgHeight = img->GetLargestPossibleRegion().GetSize()[1];

            ImageType::PointType origin = reader->GetOutput()->GetOrigin();
            m_PrimaryMissionImgOrigin[0] = origin[0];
            m_PrimaryMissionImgOrigin[1] = origin[1];
            m_strPrMissionImgProjRef = img->GetProjectionRef();
            m_GenericRSImageResampler.SetOutputProjection(m_strPrMissionImgProjRef);
            m_bValidPrimaryImg = true;
            if(oldImgProjRef != m_strPrMissionImgProjRef)
            {
                m_bUseGenericRSResampler = true;
            }
        }
    }
}

ResampleAtS2Res2::ImageType::Pointer ResampleAtS2Res2::ResampleImage(ImageType::Pointer img, Interpolator_Type interpolator,
                                                   int &nCurRes)
{
    ImageType::Pointer retImg = img;
    if(m_bValidPrimaryImg) {
        if(m_bUseGenericRSResampler)
        {
            // use the generic RS resampler that allows reprojecting
            retImg = m_GenericRSImageResampler.getResampler(img, m_scale, m_PrimaryMissionImgWidth,
                  m_PrimaryMissionImgHeight, m_PrimaryMissionImgOrigin, interpolator)
                  ->GetOutput();
        } else {
            // use the streaming resampler
            retImg = m_ImageResampler.getResampler(img, m_scale,m_PrimaryMissionImgWidth,
                  m_PrimaryMissionImgHeight, m_PrimaryMissionImgOrigin, interpolator)
                  ->GetOutput();
        }
        retImg->UpdateOutputInformation();
        // make the resolutions equal so no resampling will be done further
        nCurRes = m_nRes;
    }
    return retImg;
}
