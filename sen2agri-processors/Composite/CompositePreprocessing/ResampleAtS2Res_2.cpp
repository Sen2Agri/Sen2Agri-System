#include "ResampleAtS2Res_2.h"


ResampleAtS2Res2::ResampleAtS2Res2()
{
}

void ResampleAtS2Res2::Init(const std::string &xml, const std::string &strMaskFileName,
                            const std::string &bandsMappingFile, int nRes, const std::string &masterInfo)
{
    m_strXml = xml;
    m_strMaskFileName = strMaskFileName;
    m_nRes = nRes;

    m_Concatener = ListConcatenerFilterType::New();
    m_ImageList = InternalImageListType::New();
    m_ImageReaderList = ImageReaderListType::New();

    m_bandsMappingFile = bandsMappingFile;
    m_masterInfoFile = masterInfo;
}

void ResampleAtS2Res2::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    m_pMetadataHelper = factory->GetMetadataHelper(m_strXml);

    std::string imageFile = m_pMetadataHelper->GetImageFileName();
    m_inputImgReader = getReader(imageFile);
    m_inputImgReader->UpdateOutputInformation();
    ImageType::Pointer img = m_inputImgReader->GetOutput();
    int curRes = img->GetSpacing()[0];
    if(m_nRes <= 0) {
        m_nRes = curRes;
    }

    m_bandsCfgMappingParser.ParseFile(m_bandsMappingFile);

    std::string missionName = m_pMetadataHelper->GetMissionName();
    BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
    std::vector<int> vectIdxs = bandsMappingCfg.GetAbsoluteBandIndexes(m_nRes, missionName);
    for(unsigned int i = 0; i<vectIdxs.size(); i++) {
        int nRelBandIdx = m_pMetadataHelper->GetRelativeBandIndex(vectIdxs[i]);
        m_ImageList->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, nRelBandIdx, curRes, m_nRes, false));
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
    m_ImageAot = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, aotBandIdx, curRes, m_nRes, true);

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
    //Extract and Resample the cloud mask
    m_ImageCloud = m_ResampledBandsExtractor.ExtractResampledBand(img, 1, curRes, m_nRes, true);
    //Resample the water mask
    m_ImageWater = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, m_nRes, true);
    //Resample the snow mask
    m_ImageSnow = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, m_nRes, true);

    return true;
}

void ResampleAtS2Res2::CreateMasterInfoFile() {
    if(m_masterInfoFile != "") {
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
        std::string curMissionName = m_pMetadataHelper->GetMissionName();
        if(bandsMappingCfg.GetMasterMissionName() == curMissionName) {
            std::ofstream outFile;
            try {
              outFile.open(m_masterInfoFile.c_str());
              outFile << curMissionName << std::endl;
              outFile.close();
            } catch(...) {
              itkGenericExceptionMacro(<< "Could not open file " << outFileName);
            }
        }
    }
}
