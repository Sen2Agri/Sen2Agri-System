#include "spot4compositenaminghelper.h"

Spot4CompositeNamingHelper::Spot4CompositeNamingHelper()
{

}

bool Spot4CompositeNamingHelper::DoLoadMetadata()
{
    SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
    if (auto meta = spot4MetadataReader->ReadMetadata(m_inputMetadataFileName)) {
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

        // compute the preprocessed AOT file name
        m_PreProcessedAotFileName = getPreProcessedAotFileName(spot4Metadata);
        // compute the preprocessed Cloud file name
        m_PreProcessedCloudFileName = getPreProcessedCloudFileName(spot4Metadata);
        // compute the preprocessed Water file name
        m_PreProcessedWaterFileName = getPreProcessedWaterFileName(spot4Metadata);
        // compute the preprocessed Snow file name
        m_PreProcessedSnowFileName = getPreProcessedSnowFileName(spot4Metadata);

        // compute the AOT weight file name
        m_AotWeightFileName = GetAotWeightFileName();
        // compute the Cloud weight file name
        m_CloudWeightFileName = GetCloudWeightFileName();
        // compute the Total Weight file name
        m_TotalWeightFileName = GetTotalWeightFileName();

        return true;
    }

    return false;
}

std::string Spot4CompositeNamingHelper::AppendSuffixToTiffFileName(std::string& fileName, const std::string& suffix)
{
    return fileName.substr(0, fileName.find_last_of('.')) + suffix + ".TIF";
}

std::string Spot4CompositeNamingHelper::AppendResolutionSuffix(std::string& fileName)
{
    std::string ret;
    if(m_nResolution == 10) {
        ret = AppendSuffixToTiffFileName(fileName, "_10M");
    } else {
        ret = AppendSuffixToTiffFileName(fileName, "_20M");
    }

    return ret;
}

std::string Spot4CompositeNamingHelper::DeriveFileNameFromImageFileName(const SPOT4Metadata& spot4Metadata, const std::string& replacement)
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

std::string Spot4CompositeNamingHelper::buildFullPath(const std::string& fileName)
{
    std::string folder;
    size_t pos = m_inputMetadataFileName.find_last_of("/\\");
    if (pos == std::string::npos) {
        return fileName;
    }

    folder = m_inputMetadataFileName.substr(0, pos);
    return folder + "/" + fileName;
}

std::string Spot4CompositeNamingHelper::getImageFileName(const SPOT4Metadata& spot4Metadata) {

    return buildFullPath(spot4Metadata.Files.OrthoSurfCorrPente);
}

std::string Spot4CompositeNamingHelper::getAotFileName(const SPOT4Metadata& spot4Metadata)
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

std::string Spot4CompositeNamingHelper::getCloudFileName(const SPOT4Metadata& spot4Metadata)
{
    if(spot4Metadata.Files.MaskNua != "")
        return spot4Metadata.Files.MaskNua;
    std::string fileName = spot4Metadata.Header.Ident + "_NUA.TIF";
    return buildFullPath(fileName);
}

std::string Spot4CompositeNamingHelper::getWaterFileName(const SPOT4Metadata& spot4Metadata)
{
    if(spot4Metadata.Files.MaskDiv != "")
        return spot4Metadata.Files.MaskDiv;
    std::string fileName = spot4Metadata.Header.Ident + "_DIV.TIF";
    return buildFullPath(fileName);
}

std::string Spot4CompositeNamingHelper::getSnowFileName(const SPOT4Metadata& spot4Metadata)
{
    return getWaterFileName(spot4Metadata);
}

std::string Spot4CompositeNamingHelper::getPreProcessedAotFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string aotFile = getAotFileName(spot4Metadata);
    return AppendResolutionSuffix(aotFile);
}

std::string Spot4CompositeNamingHelper::getPreProcessedCloudFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string fileName = DeriveFileNameFromImageFileName(spot4Metadata, "CLD");
    fileName = AppendResolutionSuffix(fileName);
    return buildFullPath(fileName);
}

std::string Spot4CompositeNamingHelper::getPreProcessedWaterFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string fileName = DeriveFileNameFromImageFileName(spot4Metadata, "WATER");
    fileName = AppendResolutionSuffix(fileName);
    return buildFullPath(fileName);
}

std::string Spot4CompositeNamingHelper::getPreProcessedSnowFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string fileName = DeriveFileNameFromImageFileName(spot4Metadata, "SNOW");
    fileName = AppendResolutionSuffix(fileName);
    return buildFullPath(fileName);
}


std::string Spot4CompositeNamingHelper::getWeightAotFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string file = getPreProcessedAotFileName(spot4Metadata);
    return AppendSuffixToTiffFileName(file, "_WEIGHT");
}

std::string Spot4CompositeNamingHelper::getWeightCloudFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string file = getPreProcessedCloudFileName(spot4Metadata);
    return AppendSuffixToTiffFileName(file, "_WEIGHT");
}

std::string Spot4CompositeNamingHelper::getTotalWeightFileName(const SPOT4Metadata& spot4Metadata)
{
    std::string file = getImageFileName(spot4Metadata);
    return AppendSuffixToTiffFileName(file, "_TOTAL_WEIGHT");
}
