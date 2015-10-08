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

//    auto factory = MetadataHelperFactory::New();
//    auto pHelper = factory->GetMetadataHelper(xml);
    //std::string missionName = pHelper->GetMissionName();
}

void ResampleAtS2Res::DoExecute()
{
    std::vector<char> buf(m_strXml.begin(), m_strXml.end());
    m_DirName = std::string(dirname(buf.data()));

    auto maccsReader = itk::MACCSMetadataReader::New();
    if (auto m = maccsReader->ReadMetadata(m_strXml)) {
        ProcessLANDSAT8(m, m_bAllInOne);
    } else {
        auto spot4Reader = itk::SPOT4MetadataReader::New();
        if (auto m = spot4Reader->ReadMetadata(m_strXml)) {
            ProcessSPOT4(m, m_bAllInOne);
        }
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

bool ResampleAtS2Res::ProcessSPOT4(const std::unique_ptr<SPOT4Metadata>& meta, bool allInOne)
{
    if(meta->Radiometry.Bands.size() != 4) {
        itkExceptionMacro("Wrong number of bands for SPOT4: " + meta->Radiometry.Bands.size() );
        return false;
    }

    std::string imageFile = m_DirName + "/" + meta->Files.OrthoSurfCorrPente;
    ImageReaderType::Pointer reader = getReader(imageFile);
    reader->UpdateOutputInformation();
    ImageType::Pointer img = reader->GetOutput();
    int curRes = img->GetSpacing()[0];

    std::vector<std::string>::iterator it;
    int i = 0;
    for (it = meta->Radiometry.Bands.begin(), i = 1; it != meta->Radiometry.Bands.end(); it++, i++) {
        m_ImageListResOrig->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i));
        if(allInOne) {
            m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
        } else {
            if((*it).compare("XS1") == 0 || (*it).compare("XS2") == 0 || (*it).compare("XS3") == 0) {
                // resample to 10m
                m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            }
            else {
                if((*it).compare("SWIR") == 0) {
                    m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
                } else {
                    itkExceptionMacro("Wrong band name for SPOT4: " + (*it));
                    return false;
                }
            }
        }
    }

    // Extracts the cloud, water and snow masks resampled images
    ExtractResampledMasksImages(curRes);

    // resample the AOT
    std::string aotFileName = getSpot4AotFileName(meta);
    //otbAppLogINFO( << "AOT file name" << aotFileName << std::endl );
    ImageReaderType::Pointer aotImageReader = getReader(aotFileName);
    ImageType::Pointer aotImage = aotImageReader->GetOutput();
    m_ImageAotRes20 = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1, curRes, 20, true);
    m_ImageAotRes10 = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1, curRes, 10, true);
    m_ImageAotResOrig = m_ResampledBandsExtractor.ExtractResampledBand(aotImage, 1, curRes, -1, true);

    return true;
}

bool ResampleAtS2Res::ProcessLANDSAT8(const std::unique_ptr<MACCSFileMetadata>& meta, bool allInOne)
{
    if(meta->ImageInformation.Bands.size() != 8) {
        itkExceptionMacro("Wrong number of bands for LANDSAT: " + meta->ImageInformation.Bands.size() );
        return false;
    }
    std::string imageXMLFile("");
    std::string aotXMLFile("");
    for(auto fileInf = meta->ProductOrganization.ImageFiles.begin(); fileInf != meta->ProductOrganization.ImageFiles.end(); fileInf++)
    {
        if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_FRE") == 0 && fileInf->FileLocation.size() > 0)
            imageXMLFile = m_DirName + "/" + fileInf->FileLocation;
    }

    for(auto fileInf = meta->ProductOrganization.AnnexFiles.begin(); fileInf != meta->ProductOrganization.AnnexFiles.end(); fileInf++)
    {
        if(fileInf->File.LogicalName.substr(fileInf->File.LogicalName.size() - 4, 4).compare("_ATB") == 0 && fileInf->File.FileLocation.size() > 0)
            aotXMLFile = m_DirName + "/" + fileInf->File.FileLocation;
    }
    int nAotBandIdx = 1;
    // For MACCS, AOT is set as the band 2 in the cf. ATB file
    for (auto band : meta->ImageInformation.Bands) {
        if (band.Name == "AOT") {
            nAotBandIdx = std::stoi(band.Id);
        }
    }

    if(imageXMLFile.empty()) //TODO add error msg
        return false;

    auto maccsImageReader = itk::MACCSMetadataReader::New();
    std::unique_ptr<MACCSFileMetadata> imageMeta = nullptr;

    if ((imageMeta = maccsImageReader->ReadMetadata(imageXMLFile)) == nullptr) //TODO add error msg
        return false;

    if(imageMeta->ImageInformation.Bands.size() != 8) //TODO add error msg
        return false;

    //create image filename
    std::string imageFile = imageXMLFile;
    imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

    ImageReaderType::Pointer reader = getReader(imageFile);
    reader->UpdateOutputInformation();
    ImageType::Pointer img = reader->GetOutput();
    int curRes = img->GetSpacing()[0];

    std::vector<MACCSBand>::iterator it;
    int i = 0;
    for (it = imageMeta->ImageInformation.Bands.begin(), i = 1; it != imageMeta->ImageInformation.Bands.end(); it++, i++) {
        m_ImageListResOrig->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i));
        if(allInOne) {
            // resample to 10m
            m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));

            // resample to 20m
            m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
        } else {
            if((*it).Name.compare("B2") == 0 || (*it).Name.compare("B3") == 0 || (*it).Name.compare("B4") == 0) {
                // resample to 10m
                m_ImageListRes10->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 10, false));
            }
            else
            {
                if((*it).Name.compare("B5") == 0 || (*it).Name.compare("B7") == 0 || (*it).Name.compare("B8") == 0) {
                    // resample to 20m
                    m_ImageListRes20->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(img, i, curRes, 20, false));
                }
            }
        }
    }

    // Extracts the cloud, water and snow masks resampled images
    ExtractResampledMasksImages(curRes);

    imageFile = aotXMLFile;
    imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");
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

std::string ResampleAtS2Res::getSpot4AotFileName(const std::unique_ptr<SPOT4Metadata>& meta)
{
    // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
    // if the key is not present in the XML
    std::string fileName;
    if(meta->Files.OrthoSurfAOT == "") {
        std::string orthoSurf = meta->Files.OrthoSurfCorrPente;
        if(orthoSurf.empty()) {
            orthoSurf = meta->Files.OrthoSurfCorrEnv;
            if(!orthoSurf.empty()) {
                int nPos = orthoSurf.find("ORTHO_SURF_CORR_ENV");
                orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_ENV"), "AOT");
                fileName = orthoSurf;
            }
        } else {
            int nPos = orthoSurf.find("ORTHO_SURF_CORR_PENTE");
            orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_PENTE"), "AOT");
            fileName = orthoSurf;
        }
    } else {
        fileName = meta->Files.OrthoSurfAOT;
    }

    return m_DirName + "/" + fileName;
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
    m_ImageCloudResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 1, curRes, -1, true);

    //Resample the water mask
    m_ImageWaterRes20 = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, 20, true);
    m_ImageWaterRes10 = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, 10, true);
    m_ImageWaterResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 2, curRes, -1, true);

    //Resample the snow mask
    m_ImageSnowRes20 = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, 20, true);
    m_ImageSnowRes10 = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, 10, true);
    m_ImageSnowResOrig = m_ResampledBandsExtractor.ExtractResampledBand(img, 3, curRes, -1, true);

    return true;
}
