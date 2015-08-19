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
    if (auto meta = spot4MetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        // the helper will return the hardcoded values from the constructor as these are not
        // present in the metadata
        m_fAotQuantificationValue = 1000.0;
        m_fAotNoDataVal = 0;
        m_nAotBandIndex = 1;

        SPOT4Metadata spot4Metadata = *meta;
        // compute the Image file name
        m_ImageFileName = getImageFileName(spot4Metadata);

        // compute the AOT file name
        m_AotFileName = getAotFileName(spot4Metadata);
        // compute the Cloud file name
        m_CloudFileName = getCloudFileName(spot4Metadata);
        // compute the Water file name
        m_WaterFileName = getWaterFileName(spot4Metadata);
        // compute the Snow file name
        m_SnowFileName = getSnowFileName(spot4Metadata);

        return true;
    }

    return false;
}

std::string Spot4MetadataHelper::DeriveFileNameFromImageFileName(const SPOT4Metadata& spot4Metadata, const std::string& replacement)
{
    std::string fileName;
    std::string orthoSurf = spot4Metadata.Files.OrthoSurfCorrPente;
    if(orthoSurf.empty()) {
        orthoSurf = spot4Metadata.Files.OrthoSurfCorrEnv;
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

std::string Spot4MetadataHelper::getImageFileName(const SPOT4Metadata& spot4Metadata) {

    return buildFullPath(spot4Metadata.Files.OrthoSurfCorrPente);
}

std::string Spot4MetadataHelper::getAotFileName(const SPOT4Metadata& spot4Metadata)
{
    // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
    // if the key is not present in the XML
    std::string fileName;
    if(spot4Metadata.Files.OrthoSurfAOT == "") {
        fileName = DeriveFileNameFromImageFileName(spot4Metadata, "AOT");
    } else {
        fileName = spot4Metadata.Files.OrthoSurfAOT;
    }

    return buildFullPath(fileName);
}

std::string Spot4MetadataHelper::getCloudFileName(const SPOT4Metadata& spot4Metadata)
{
    if(spot4Metadata.Files.MaskNua != "")
        return spot4Metadata.Files.MaskNua;
    std::string fileName = spot4Metadata.Header.Ident + "_NUA.TIF";
    return buildFullPath(fileName);
}

std::string Spot4MetadataHelper::getWaterFileName(const SPOT4Metadata& spot4Metadata)
{
    if(spot4Metadata.Files.MaskDiv != "")
        return spot4Metadata.Files.MaskDiv;
    std::string fileName = spot4Metadata.Header.Ident + "_DIV.TIF";
    return buildFullPath(fileName);
}

std::string Spot4MetadataHelper::getSnowFileName(const SPOT4Metadata& spot4Metadata)
{
    return getWaterFileName(spot4Metadata);
}


