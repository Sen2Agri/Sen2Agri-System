#include "Spot4MetadataHelper.h"

Spot4MetadataHelper::Spot4MetadataHelper()
{
    m_fAotQuantificationValue = 1000.0;
    m_fAotNoDataVal = 0;
    m_nAotBandIndex = 1;
}

bool Spot4MetadataHelper::DoLoadMetadata()
{
    SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
    if (m_metadata = spot4MetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        // the helper will return the hardcoded values from the constructor as these are not
        // present in the metadata
        m_fAotQuantificationValue = 1000.0;
        m_fAotNoDataVal = 0;
        m_nAotBandIndex = 1;

        // For Spot4 the bands are XS1;XS2;XS3;SWIR that correspond to RED, GREEN, NIR and SWIR
        m_nRedBandIndex = 1;
        m_nGreenBandIndex = 2;
        m_nNirBandIndex = 3;

        m_ReflQuantifVal = 1000.0;

        m_Mission = "SPOT4";
        // compute the Image file name
        m_ImageFileName = getImageFileName();

        // compute the AOT file name
        m_AotFileName = getAotFileName();
        // compute the Cloud file name
        m_CloudFileName = getCloudFileName();
        // compute the Water file name
        m_WaterFileName = getWaterFileName();
        // compute the Snow file name
        m_SnowFileName = getSnowFileName();

        // extract the acquisition date
        m_AcquisitionDate = m_metadata->Header.DatePdv.substr(0,4) +
                m_metadata->Header.DatePdv.substr(5,2) + m_metadata->Header.DatePdv.substr(8,2);

        return true;
    }

    return false;
}

std::string Spot4MetadataHelper::DeriveFileNameFromImageFileName(const std::string& replacement)
{
    std::string fileName;
    std::string orthoSurf = m_metadata->Files.OrthoSurfCorrPente;
    if(orthoSurf.empty()) {
        orthoSurf = m_metadata->Files.OrthoSurfCorrEnv;
        if(!orthoSurf.empty()) {
            int nPos = orthoSurf.find("ORTHO_SURF_CORR_ENV");
            orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_ENV"), replacement);
            fileName = orthoSurf;
        }
    } else {
        int nPos = orthoSurf.find("ORTHO_SURF_CORR_PENTE");
        orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_PENTE"), replacement);
        fileName = orthoSurf;
    }

    return fileName;
}

std::string Spot4MetadataHelper::buildFullPath(const std::string& fileName)
{
    std::string folder;
    size_t pos = m_inputMetadataFileName.find_last_of("/\\");
    if (pos == std::string::npos) {
        return fileName;
    }

    folder = m_inputMetadataFileName.substr(0, pos);
    return folder + "/" + fileName;
}

std::string Spot4MetadataHelper::getImageFileName() {

    return buildFullPath(m_metadata->Files.OrthoSurfCorrPente);
}

std::string Spot4MetadataHelper::getAotFileName()
{
    // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
    // if the key is not present in the XML
    std::string fileName;
    if(m_metadata->Files.OrthoSurfAOT == "") {
        fileName = DeriveFileNameFromImageFileName("AOT");
    } else {
        fileName = m_metadata->Files.OrthoSurfAOT;
    }

    return buildFullPath(fileName);
}

std::string Spot4MetadataHelper::getCloudFileName()
{
    return buildFullPath(m_metadata->Files.MaskNua);
}

std::string Spot4MetadataHelper::getWaterFileName()
{
    return buildFullPath(m_metadata->Files.MaskDiv);
}

std::string Spot4MetadataHelper::getSnowFileName()
{
    return getWaterFileName();
}


