#include "MACCSMetadataHelper.h"

#define LANDSAT_STR    "LANDSAT"
#define SENTINEL_STR   "SENTINEL"

MACCSMetadataHelper::MACCSMetadataHelper()
{
    m_missionType = S2;
}

bool MACCSMetadataHelper::DoLoadMetadata()
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    // just check if the file is Spot4 metadata file. In this case
    // the helper will return the hardcoded values from the constructor as these are not
    // present in the metadata
    if (auto metadata = maccsMetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        MACCSFileMetadata maccsMetadata = *metadata;

        if (maccsMetadata.Header.FixedHeader.Mission.find(LANDSAT_STR) != std::string::npos) {
            m_missionType = LANDSAT;
            UpdateValuesForLandsat(maccsMetadata);
        } else if (maccsMetadata.Header.FixedHeader.Mission.find(SENTINEL_STR) != std::string::npos) {
            m_missionType = S2;
            UpdateValuesForSentinel(maccsMetadata);
        } else {
            itkExceptionMacro("Unknown mission: " + maccsMetadata.Header.FixedHeader.Mission);
        }

        m_Mission = maccsMetadata.Header.FixedHeader.Mission;

        // compute the Image file name
        m_ImageFileName = getImageFileName(maccsMetadata);

        // compute the AOT file name
        m_AotFileName = getAotFileName(maccsMetadata);
        // compute the Cloud file name
        m_CloudFileName = getCloudFileName(maccsMetadata);
        // compute the Water file name
        m_WaterFileName = getWaterFileName(maccsMetadata);
        // compute the Snow file name
        m_SnowFileName = getSnowFileName(maccsMetadata);
        // set the acquisition date
        m_AcquisitionDate = maccsMetadata.InstanceId.AcquisitionDate;

        return true;
    }

    return false;
}

void MACCSMetadataHelper::UpdateValuesForLandsat(const MACCSFileMetadata& meta)
{
    std::string specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB");
    ReadSpecificMACCSHdrFile(specHdrFile);
}

void MACCSMetadataHelper::UpdateValuesForSentinel(const MACCSFileMetadata& meta)
{
    std::string specHdrFile;
    if(m_nResolution == 10) {
        specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB_R1");
    } else {
        specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB_R2");
    }
    ReadSpecificMACCSHdrFile(specHdrFile);
}

void MACCSMetadataHelper::ReadSpecificMACCSHdrFile(const std::string& fileName)
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    if (auto meta = maccsMetadataReader->ReadMetadata(fileName)) {
        // add the information to the list
        MACCSFileMetadata maccsMetadata = *meta;
        m_fAotQuantificationValue = atof(maccsMetadata.ImageInformation.AOTQuantificationValue.c_str());
        m_fAotNoDataVal = atof(maccsMetadata.ImageInformation.AOTNoDataValue.c_str());
        m_nAotBandIndex = getBandIndex(maccsMetadata.ImageInformation.Bands, "AOT");
    }
}

std::string MACCSMetadataHelper::getImageFileName(const MACCSFileMetadata& meta) {
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_FRE_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_FRE_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_FRE");
    }
}

std::string MACCSMetadataHelper::getAotFileName(const MACCSFileMetadata& meta)
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_ATB");
    }
}

std::string MACCSMetadataHelper::getCloudFileName(const MACCSFileMetadata& meta)
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_CLD_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_CLD_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, meta.ProductOrganization.AnnexFiles, "_CLD");
    }
}

std::string MACCSMetadataHelper::getWaterFileName(const MACCSFileMetadata& meta)
{
    // TODO:
    return "";
}

std::string MACCSMetadataHelper::getSnowFileName(const MACCSFileMetadata &meta)
{
    // TODO:
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageFileName(const std::string& descriptor,
                                                          const std::vector<MACCSFileInformation>& imageFiles,
                                                           const std::string& ending) {

    std::string folder;
    size_t pos = descriptor.find_last_of("/\\");
    if (pos == std::string::npos) {
        folder = "";
    } else {
        folder = descriptor.substr(0, pos);
    }

    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return folder + "/" + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
        }

    }
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageFileName(const std::string& descriptor,
                                                           const std::vector<MACCSAnnexInformation>& maskFiles,
                                                           const std::string& ending) {

    std::string folder;
    size_t pos = descriptor.find_last_of("/\\");
    if (pos == std::string::npos) {
        folder = "";
    } else {
        folder = descriptor.substr(0, pos);
    }

    for (const MACCSAnnexInformation& fileInfo : maskFiles) {
        if (fileInfo.File.LogicalName.length() >= ending.length() &&
                0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return folder + "/" + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
        }

    }
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageHdrName(const std::string& descriptor, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

    std::string folder;
    size_t pos = descriptor.find_last_of("/\\");
    if (pos == std::string::npos) {
        folder = "";
    } else {
        folder = descriptor.substr(0, pos);
    }

    for (const MACCSAnnexInformation& fileInfo : maskFiles) {
        if (fileInfo.File.LogicalName.length() >= ending.length() &&
                0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return folder + "/" + fileInfo.File.FileLocation;
        }

    }
    return "";
}

// Get the id of the band. Return -1 if band not found.
int MACCSMetadataHelper::getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name) {
    for (const MACCSBand& band : bands) {
        if (band.Name == name) {
            return std::stoi(band.Id);
        }
    }
    return -1;
}

