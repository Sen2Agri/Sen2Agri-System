#include "MACCSMetadataHelper.h"

MACCSMetadataHelper::MACCSMetadataHelper()
{
    m_missionType = S2;
    m_ReflQuantifVal = 1;
}

bool MACCSMetadataHelper::DoLoadMetadata()
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    // just check if the file is Spot4 metadata file. In this case
    // the helper will return the hardcoded values from the constructor as these are not
    // present in the metadata
    if (m_metadata = maccsMetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        if (m_metadata->Header.FixedHeader.Mission.find(LANDSAT_MISSION_STR) != std::string::npos) {
            m_missionType = LANDSAT;
            m_nTotalBandsNo = 6;
            UpdateValuesForLandsat();
        } else if (m_metadata->Header.FixedHeader.Mission.find(SENTINEL_MISSION_STR) != std::string::npos) {
            m_missionType = S2;
            m_nTotalBandsNo = 10;
            UpdateValuesForSentinel();
        } else {
            itkExceptionMacro("Unknown mission: " + m_metadata->Header.FixedHeader.Mission);
        }

        m_Mission = m_metadata->Header.FixedHeader.Mission;
        //m_ReflQuantifVal = std::stod(maccsMetadata.ProductInformation.ReflectanceQuantificationValue);

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
        // set the acquisition date
        m_AcquisitionDate = m_metadata->InstanceId.AcquisitionDate;

        if(m_nResolution == 10)
        {
            m_nRedBandIndex = getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B4");
            m_nGreenBandIndex = getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B3");
            m_nNirBandIndex = getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B8");
        }

        //TODO: Add initialization for mean angles (solar and sensor)

        return true;
    }

    return false;
}

void MACCSMetadataHelper::UpdateValuesForLandsat()
{
    std::string specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB");
    ReadSpecificMACCSHdrFile(specHdrFile);
}

void MACCSMetadataHelper::UpdateValuesForSentinel()
{
    std::string specHdrFile;
    if(m_nResolution == 10) {
        specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
    } else {
        specHdrFile = getMACCSImageHdrName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
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

std::string MACCSMetadataHelper::getImageFileName() {
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_FRE_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_FRE_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_FRE");
    }
}

std::string MACCSMetadataHelper::getAotFileName()
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_ATB");
    }
}

std::string MACCSMetadataHelper::getCloudFileName()
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_CLD_R1");
        } else {
            return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_CLD_R2");
        }
    } else {
        return getMACCSImageFileName(m_inputMetadataFileName, m_metadata->ProductOrganization.AnnexFiles, "_CLD");
    }
}

std::string MACCSMetadataHelper::getWaterFileName()
{
    // TODO:
    return "";
}

std::string MACCSMetadataHelper::getSnowFileName()
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

