#include "MACCSMetadataHelper.h"

MACCSMetadataHelper::MACCSMetadataHelper()
{
    m_missionType = S2;
    m_ReflQuantifVal = 1;
}
/*
std::string MACCSMetadataHelper::GetBandName(unsigned int nBandIdx, bool bRelativeIdx)
{
    if(bRelativeIdx) {
        if(!m_specificImgMetadata) {
            ReadSpecificMACCSImgHdrFile();
        }

        if(nBandIdx >= m_nBandsNoForCurRes) {
            itkExceptionMacro("Invalid band index requested: " << bRelativeIdx << ". Maximum is " << m_nBandsNoForCurRes);
        }
        return m_specificImgMetadata->ImageInformation.Bands[nBandIdx].Name;
    } else {
        for (const MACCSBand& band : m_metadata->ImageInformation.Bands) {
            // the bands in the file are 1 based while our parameter nBandIdx is 0 based
            if (std::stoi(band.Id) == (int)nBandIdx) {
                return band.Name;
            }
        }
        itkExceptionMacro("Invalid absolute band index requested: " << nBandIdx << ". Maximum is " << m_nTotalBandsNo);
    }
}
*/

std::string MACCSMetadataHelper::GetBandName(unsigned int nBandIdx, bool bRelativeIdx)
{
    if(bRelativeIdx) {
        if(!m_specificImgMetadata) {
            ReadSpecificMACCSImgHdrFile();
        }

        if(nBandIdx >= m_nBandsNoForCurRes) {
            itkExceptionMacro("Invalid band index requested: " << bRelativeIdx << ". Maximum is " << m_nBandsNoForCurRes);
        }
        return m_specificImgMetadata->ImageInformation.Bands[nBandIdx].Name;
    } else {
        if(m_missionType == LANDSAT) {
            for (const MACCSBand& band : m_metadata->ImageInformation.Bands) {
                // the bands in the file are 1 based while our parameter nBandIdx is 0 based
                if (std::stoi(band.Id) == (int)nBandIdx) {
                    return band.Name;
                }
            }
        } else {
            // Sentinel 2
            for(const MACCSResolution& maccsRes: m_metadata->ImageInformation.Resolutions) {
                if(std::atoi(maccsRes.Id.c_str()) == m_nResolution) {
                    for (const MACCSBand& band : maccsRes.Bands) {
                        // the bands in the file are 1 based while our parameter nBandIdx is 0 based
                        if (std::stoi(band.Id) == (int)nBandIdx) {
                            return band.Name;
                        }
                    }
                }
            }
        }
        itkExceptionMacro("Invalid absolute band index requested: " << nBandIdx << ". Maximum is " << m_nTotalBandsNo);
    }
}

int MACCSMetadataHelper::GetRelativeBandIndex(unsigned int nAbsBandIdx)
{
    if(m_missionType == LANDSAT) {
        return nAbsBandIdx;
    }
    // In the case of S2 we need to compute the relative index
    std::string bandName = GetBandName(nAbsBandIdx, false);
    if(!m_specificImgMetadata) {
        ReadSpecificMACCSImgHdrFile();
    }
    return getBandIndex(m_specificImgMetadata->ImageInformation.Bands, bandName);
}

float MACCSMetadataHelper::GetAotQuantificationValue()
{
    if(!m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    return m_fAotQuantificationValue;
}

float MACCSMetadataHelper::GetAotNoDataValue()
{
    if(!m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    return m_fAotNoDataVal;
}

int MACCSMetadataHelper::GetAotBandIndex()
{
    if(!m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    return m_nAotBandIndex;
}


bool MACCSMetadataHelper::DoLoadMetadata()
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    // just check if the file is Spot4 metadata file. In this case
    // the helper will return the hardcoded values from the constructor as these are not
    // present in the metadata
    if (m_metadata = maccsMetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        if (m_metadata->Header.FixedHeader.Mission.find(LANDSAT_MISSION_STR) != std::string::npos) {
            UpdateValuesForLandsat();
        } else if (m_metadata->Header.FixedHeader.Mission.find(SENTINEL_MISSION_STR) != std::string::npos) {
            UpdateValuesForSentinel();
        } else {
            itkExceptionMacro("Unknown mission: " + m_metadata->Header.FixedHeader.Mission);
        }

        m_Mission = m_metadata->Header.FixedHeader.Mission;
        m_ReflQuantifVal = std::stod(m_metadata->ProductInformation.ReflectanceQuantificationValue);

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

        //TODO: Add initialization for mean angles (solar and sensor)

        return true;
    }

    return false;
}

void MACCSMetadataHelper::UpdateValuesForLandsat()
{
    m_missionType = LANDSAT;
    m_nTotalBandsNo = 6;
    m_nBandsNoForCurRes = m_nTotalBandsNo;
    m_nRedBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B4");
    m_nBlueBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B2");
    m_nGreenBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B3");
    m_nNirBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B5");
    // TODO: Add here other initializations for LANDSAT if needed
}

void MACCSMetadataHelper::UpdateValuesForSentinel()
{
    m_missionType = S2;
    m_nTotalBandsNo = 10;
    m_nBandsNoForCurRes = ((m_nResolution == 10) ? 4 : 6);
    if(m_nResolution == 10)
    {
        m_nRedBandIndex = GetRelativeBandIndex(getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B4"));
        m_nBlueBandIndex = GetRelativeBandIndex(getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B2"));
        m_nGreenBandIndex = GetRelativeBandIndex(getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B3"));
        m_nNirBandIndex = GetRelativeBandIndex(getBandIndex(m_metadata->ImageInformation.Resolutions[0].Bands, "B8"));
    }
    // TODO: Add here other initializations for S2 if needed
}

void MACCSMetadataHelper::ReadSpecificMACCSImgHdrFile()
{
    std::string fileName;
    if(m_missionType == LANDSAT) {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.ImageFiles, "_FRE");
    } else {
        if(m_nResolution == 10) {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.ImageFiles, "_FRE_R1");
        } else {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.ImageFiles, "_FRE_R1");
        }
    }

    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    m_specificImgMetadata = maccsMetadataReader->ReadMetadata(fileName);
    if(m_nBandsNoForCurRes > m_specificImgMetadata->ImageInformation.Bands.size()) {
        itkExceptionMacro("Invalid number of bands found in specific img xml: " <<
                          m_specificImgMetadata->ImageInformation.Bands.size() <<
                          ". Expected is " << m_nBandsNoForCurRes);
    }
}

void MACCSMetadataHelper::ReadSpecificMACCSCldHdrFile()
{
    std::string fileName;
    if(m_missionType == LANDSAT) {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_CLD");
    } else {
        if(m_nResolution == 10) {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_CLD_R1");
        } else {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_CLD_R2");
        }
    }

    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    m_specificCldMetadata = maccsMetadataReader->ReadMetadata(fileName);
}

void MACCSMetadataHelper::ReadSpecificMACCSAotHdrFile()
{
    std::string fileName;
    if(m_missionType == LANDSAT) {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_ATB");
    } else {
        if(m_nResolution == 10) {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
        } else {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
        }
    }
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    if (m_specificAotMetadata = maccsMetadataReader->ReadMetadata(fileName)) {
        // add the information to the list
        m_fAotQuantificationValue = atof(m_specificAotMetadata->ImageInformation.AOTQuantificationValue.c_str());
        m_fAotNoDataVal = atof(m_specificAotMetadata->ImageInformation.AOTNoDataValue.c_str());
        m_nAotBandIndex = getBandIndex(m_specificAotMetadata->ImageInformation.Bands, "AOT");
    }
}

void MACCSMetadataHelper::ReadSpecificMACCSMskHdrFile()
{
    std::string fileName;
    if(m_missionType == LANDSAT) {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK");
    } else {
        if(m_nResolution == 10) {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R1");
        } else {
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R1");
        }
    }

    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    m_specificMskMetadata = maccsMetadataReader->ReadMetadata(fileName);
}

std::string MACCSMetadataHelper::getImageFileName() {
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_metadata->ProductOrganization.ImageFiles, "_FRE_R1");
        } else {
            return getMACCSImageFileName(m_metadata->ProductOrganization.ImageFiles, "_FRE_R2");
        }
    } else {
        return getMACCSImageFileName(m_metadata->ProductOrganization.ImageFiles, "_FRE");
    }
}

std::string MACCSMetadataHelper::getAotFileName()
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
        } else {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
        }
    } else {
        return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_ATB");
    }
}

std::string MACCSMetadataHelper::getCloudFileName()
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_CLD_R1");
        } else {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_CLD_R2");
        }
    } else {
        return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_CLD");
    }
}

std::string MACCSMetadataHelper::getWaterFileName()
{
    if(m_missionType == S2) {
        if(m_nResolution == 10) {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R1");
        } else {
            return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R2");
        }
    } else {
        return getMACCSImageFileName(m_metadata->ProductOrganization.AnnexFiles, "_MSK");
    }
}

std::string MACCSMetadataHelper::getSnowFileName()
{
    // the water and snow masks are in the same file
    return getWaterFileName();
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageFileName(const std::vector<MACCSFileInformation>& imageFiles,
                                                       const std::string& ending) {
    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return m_DirName + "/" + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
        }
    }
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageFileName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                                       const std::string& ending) {
    for (const MACCSAnnexInformation& fileInfo : maskFiles) {
        if (fileInfo.File.LogicalName.length() >= ending.length() &&
                0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return m_DirName + "/" + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
        }
    }
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageHdrName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                                      const std::string& ending) {

    for (const MACCSAnnexInformation& fileInfo : maskFiles) {
        if (fileInfo.File.LogicalName.length() >= ending.length() &&
                0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return m_DirName + "/" + fileInfo.File.FileLocation;
        }
    }
    return "";
}

// Return the path to a file for which the name end in the ending
std::string MACCSMetadataHelper::getMACCSImageHdrName(const std::vector<MACCSFileInformation>& imageFiles,
                                                      const std::string& ending) {

    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return m_DirName + "/" + fileInfo.FileLocation;
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

