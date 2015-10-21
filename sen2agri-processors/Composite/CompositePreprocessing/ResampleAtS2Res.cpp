#include "ResampleAtS2Res.h"

ResampleAtS2Res::ResampleAtS2Res()
{
}

void ResampleAtS2Res::Init(bool bAllInOne, std::string &xml, std::string &strMaskFileName, int nRes)
{
    m_bAllInOne = bAllInOne;
    m_strXml = xml;
    m_strMaskFileName = strMaskFileName;
    m_nRes = nRes;

    m_ConcatenerRes10 = ListConcatenerFilterType::New();
    m_ConcatenerRes20 = ListConcatenerFilterType::New();
    m_ConcatenerResOrig = ListConcatenerFilterType::New();

    m_ImageListRes10 = InternalImageListType::New();
    m_ImageListRes20 = InternalImageListType::New();
    m_ImageListResOrig = InternalImageListType::New();

    m_ImageReaderList = ImageReaderListType::New();

    auto factory = MetadataHelperFactory::New();
    m_pMetadataHelper = factory->GetMetadataHelper(m_strXml);
}

void ResampleAtS2Res::DoExecute()
{
    std::string missionName = m_pMetadataHelper->GetMissionName();
    if (missionName.find(LANDSAT_MISSION_STR) != std::string::npos) {
        ProcessLANDSAT8(m_bAllInOne);
    } else if(missionName.find(SPOT4_MISSION_STR) != std::string::npos) {
        ProcessSPOT4(m_bAllInOne);
    }

    m_ConcatenerRes10->SetInput( m_ImageListRes10 );
    m_ConcatenerRes20->SetInput( m_ImageListRes20 );
    m_ConcatenerResOrig->SetInput( m_ImageListResOrig );
}

ResampleAtS2Res::ImageType::Pointer ResampleAtS2Res::GetResampledMainImg() {
    if(m_nRes == 10) {
        return m_ConcatenerRes10->GetOutput();
    } else if(m_nRes == 20) {
        return m_ConcatenerRes20->GetOutput();
    } else {
        return m_ConcatenerResOrig->GetOutput();
    }
}

ResampleAtS2Res::InternalImageType::Pointer ResampleAtS2Res::GetResampledCloudMaskImg() {
    if(m_nRes == 10) {
        return m_ImageCloudRes10;
    } else if(m_nRes == 20) {
        return m_ImageCloudRes20;
    } else {
        return m_ImageCloudResOrig;
    }
}

ResampleAtS2Res::InternalImageType::Pointer ResampleAtS2Res::GetResampledWaterMaskImg() {
    if(m_nRes == 10) {
        return m_ImageWaterRes10;
    } else if(m_nRes == 20) {
        return m_ImageWaterRes20;
    } else {
        return m_ImageWaterResOrig;
    }
}

ResampleAtS2Res::InternalImageType::Pointer ResampleAtS2Res::GetResampledSnowMaskImg() {
    if(m_nRes == 10) {
        return m_ImageSnowRes10;
    } else if(m_nRes == 20) {
        return m_ImageSnowRes20;
    } else {
        return m_ImageSnowResOrig;
    }
}

ResampleAtS2Res::InternalImageType::Pointer ResampleAtS2Res::GetResampledAotImg() {
    if(m_nRes == 10) {
        return m_ImageAotRes10;
    } else if(m_nRes == 20) {
        return m_ImageAotRes20;
    } else {
        return m_ImageAotResOrig;
    }
}

bool ResampleAtS2Res::ProcessSPOT4(bool allInOne)
{
    std::string imageFile = m_pMetadataHelper->GetImageFileName();
    ImageReaderType::Pointer reader = getReader(imageFile);
    reader->UpdateOutputInformation();
    ImageType::Pointer img = reader->GetOutput();
    int curRes = img->GetSpacing()[0];

    int nBandsNo = m_pMetadataHelper->GetBandsNoForCurrentResolution();
    for (int i = 1; i<=nBandsNo; i++) {
        m_ImageListResOrig->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i));
        if(allInOne) {
            m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
        } else {
            std::string bandName = m_pMetadataHelper->GetBandName(i-1);
            if(bandName.compare("XS1") == 0 || bandName.compare("XS2") == 0 || bandName.compare("XS3") == 0) {
                // resample to 10m
                m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            }
            else {
                if(bandName.compare("SWIR") == 0) {
                    m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
                } else {
                    itkExceptionMacro("Wrong band name for SPOT4: " + bandName);
                    return false;
                }
            }
        }
    }

    // Extracts the cloud, water and snow masks resampled images
    ExtractResampledMasksImages(curRes);

    // resample the AOT
    std::string aotFileName = m_pMetadataHelper->GetAotImageFileName();
    //otbAppLogINFO( << "AOT file name" << aotFileName << std::endl );
    ImageReaderType::Pointer aotImageReader = getReader(aotFileName);
    ImageType::Pointer aotImage = aotImageReader->GetOutput();
    m_ImageAotRes20 = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1, curRes, 20, true);
    m_ImageAotRes10 = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1, curRes, 10, true);
    m_ImageAotResOrig = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1);

    return true;
}

bool ResampleAtS2Res::ProcessLANDSAT8(bool allInOne)
{
    //create image filename
    std::string imageFile = m_pMetadataHelper->GetImageFileName();

    ImageReaderType::Pointer reader = getReader(imageFile);
    reader->UpdateOutputInformation();
    ImageType::Pointer img = reader->GetOutput();
    int curRes = img->GetSpacing()[0];

    int nBandsNo = m_pMetadataHelper->GetBandsNoForCurrentResolution();
    for (int i = 1; i<=nBandsNo; i++) {
        m_ImageListResOrig->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i));
        if(allInOne) {
            // resample to 10m
            m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            // resample to 20m
            m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
        } else {
            std::string bandName = m_pMetadataHelper->GetBandName(i-1);
            if(bandName.compare("B2") == 0 || bandName.compare("B3") == 0 || bandName.compare("B4") == 0) {
                // resample to 10m
                m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            }
            else
            {
                if(bandName.compare("B5") == 0 || bandName.compare("B7") == 0 || bandName.compare("B8") == 0) {
                    // resample to 20m
                    m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
                }
            }
        }
    }

    // Extracts the cloud, water and snow masks resampled images
    ExtractResampledMasksImages(curRes);

    imageFile = m_pMetadataHelper->GetAotImageFileName();
    int nAotBandIdx = m_pMetadataHelper->GetAotBandIndex();
    ImageReaderType::Pointer readerAot = getReader(imageFile);
    ImageType::Pointer imgAot = readerAot->GetOutput();
    m_ImageAotRes20 = m_ResampledBandsExtractor.ExtractResampledBand(imgAot, nAotBandIdx, curRes, 20, true);
    m_ImageAotRes10 = m_ResampledBandsExtractor.ExtractResampledBand(imgAot, nAotBandIdx, curRes, 10, true);
    m_ImageAotResOrig = m_ResampledBandsExtractor.ExtractResampledBand(imgAot, nAotBandIdx);

    return true;
}

// get a reader from the file path
ResampleAtS2Res::ImageReaderType::Pointer ResampleAtS2Res::getReader(const std::string& filePath) {
    ImageReaderType::Pointer reader = ImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath);

    // add it to the list and return
    m_ImageReaderList->PushBack(reader);
    return reader;
}

bool ResampleAtS2Res::ExtractResampledMasksImages(int curRes)
{
    if(m_strMaskFileName == "") {
        itkExceptionMacro("The mask file for SPOT was not provided, do set 'spotmask' flag ");
        return false;
    }
    ImageReaderType::Pointer masksReader = getReader(m_strMaskFileName);
    ImageType::Pointer img = masksReader->GetOutput();
    //Extract and Resample the cloud mask
    m_ImageCloudRes20 = m_ResampledBandsExtractor.ExtractResampledBand(img, 1, curRes, 20, true);
    m_ImageCloudRes10 = m_ResampledBandsExtractor.ExtractResampledBand(img, 1, curRes, 10, true);
    m_ImageCloudResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 1);

    //Resample the water mask
    m_ImageWaterRes20 = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, 20, true);
    m_ImageWaterRes10 = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, 10, true);
    m_ImageWaterResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 2);

    //Resample the snow mask
    m_ImageSnowRes20 = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, 20, true);
    m_ImageSnowRes10 = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, 10, true);
    m_ImageSnowResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 3);

    return true;
}
