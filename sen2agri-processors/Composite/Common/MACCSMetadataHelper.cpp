#include "MACCSMetadataHelper.h"
#include "ViewingAngles.hpp"

MACCSMetadataHelper::MACCSMetadataHelper()
{
    m_missionType = S2;
    m_ReflQuantifVal = 1;
}

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
        const std::vector<MACCSBand>& maccsBands = GetAllMACCSBandsInfos();
        for (const MACCSBand& band : maccsBands) {
            // the bands in the file are 1 based while our parameter nBandIdx is 0 based
            if (std::stoi(band.Id) == (int)nBandIdx) {
                return band.Name;
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
        m_strNoDataValue = m_metadata->ImageInformation.NoDataValue;

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
    // we have the same values for relative and absolute bands indexes as we have only one raster
    m_nAbsRedBandIndex = m_nRelRedBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B4");
    m_nAbsBlueBandIndex = m_nRelBlueBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B2");
    m_nAbsGreenBandIndex = m_nRelGreenBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B3");
    m_nAbsNirBandIndex = m_nRelNirBandIndex = getBandIndex(m_metadata->ImageInformation.Bands, "B5");
    
    m_bHasGlobalMeanAngles = true;
    m_bHasBandMeanAngles = false;

    // update the solar mean angle
    m_solarMeanAngles.azimuth = m_metadata->ProductInformation.MeanSunAngle.AzimuthValue;
    m_solarMeanAngles.zenith = m_metadata->ProductInformation.MeanSunAngle.ZenithValue;

    // update the solar mean angle
    //MeanAngles_Type sensorAngle;
    // TODO: Here we should get this from the Mean_Viewing_Angle. Most probably if we get here it will crash
    // if the MACCS Metadata Reader will not be updated to read this element for LANDSAT8
    //sensorAngle.azimuth = m_metadata->ProductInformation.MeanViewingIncidenceAngles.at(0).Angles.AzimuthValue;
    //sensorAngle.zenith = m_metadata->ProductInformation.MeanViewingIncidenceAngles.at(0).Angles.ZenithValue;
    //m_sensorBandsMeanAngles.push_back(sensorAngle);

    
    // TODO: Add here other initializations for LANDSAT if needed
}

void MACCSMetadataHelper::UpdateValuesForSentinel()
{
    if(m_nResolution != 10 && m_nResolution != 20) {
        itkExceptionMacro("Accepted resolutions for Sentinel mission are 10 or 20 only!");
    }
    m_missionType = S2;
    m_nTotalBandsNo = 10;
    m_nBandsNoForCurRes = ((m_nResolution == 10) ? 4 : 6);
    if(m_nResolution == 10)
    {
        // for S2 we do not use the ImageInformation bands but the bands from the MACCSResolution structures
        //const MACCSResolution& maccsRes = GetMACCSResolutionInfo(m_nResolution);
        std::vector<MACCSBand> maccsBands = GetAllMACCSBandsInfos();
        m_nAbsRedBandIndex = getBandIndex(maccsBands, "B4");
        m_nAbsBlueBandIndex = getBandIndex(maccsBands, "B2");
        m_nAbsGreenBandIndex = getBandIndex(maccsBands, "B3");
        m_nAbsNirBandIndex = getBandIndex(maccsBands, "B8");

        m_nRelRedBandIndex = GetRelativeBandIndex(m_nAbsRedBandIndex);
        m_nRelBlueBandIndex = GetRelativeBandIndex(m_nAbsBlueBandIndex);
        m_nRelGreenBandIndex = GetRelativeBandIndex(m_nAbsGreenBandIndex);
        m_nRelNirBandIndex = GetRelativeBandIndex(m_nAbsNirBandIndex);
    }
    
    InitializeS2Angles();

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
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.ImageFiles, "_FRE_R2");
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
            fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R2");
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

const MACCSResolution& MACCSMetadataHelper::GetMACCSResolutionInfo(int nResolution) {
    for(const MACCSResolution& maccsRes: m_metadata->ImageInformation.Resolutions) {
        if(std::atoi(maccsRes.Id.c_str()) == nResolution) {
            return maccsRes;
        }
    }
    itkExceptionMacro("No resolution structure was found in the main xml for S2 resolution : " << nResolution);
}

std::vector<MACCSBand> MACCSMetadataHelper::GetAllMACCSBandsInfos() {
    if(m_missionType == LANDSAT) {
        return m_metadata->ImageInformation.Bands;
    } else {
        // Sentinel 2
        std::vector<MACCSBand> maccsBands;
        int bandIdx = 1;
        for(const MACCSBandWavelength& maccsBandWavelength: m_metadata->ProductInformation.BandWavelengths) {
            MACCSBand band;
            band.Id = std::to_string(bandIdx);
            band.Name = maccsBandWavelength.BandName;
            maccsBands.push_back(band);
            bandIdx++;
        }
        return maccsBands;
    }
}

void MACCSMetadataHelper::InitializeS2Angles() {
    m_bHasGlobalMeanAngles = true;
    m_bHasBandMeanAngles = true;
    m_detailedAnglesGridSize = 23;

    // update the solar mean angle
    m_solarMeanAngles.azimuth = m_metadata->ProductInformation.MeanSunAngle.AzimuthValue;
    m_solarMeanAngles.zenith = m_metadata->ProductInformation.MeanSunAngle.ZenithValue;

    // first compute the total number of bands to add into m_sensorBandsMeanAngles
    unsigned int nMaxBandId = 0;
    std::vector<MACCSMeanViewingIncidenceAngle> angles = m_metadata->ProductInformation.MeanViewingIncidenceAngles;
    for(unsigned int i = 0; i<angles.size(); i++) {
        unsigned int anglesBandId = std::atoi(angles[i].BandId.c_str());
        if(nMaxBandId < anglesBandId) {
            nMaxBandId = anglesBandId;
        }
    }
    if(nMaxBandId+1 != angles.size()) {
        std::cout << "ATTENTION: Number of mean viewing angles different than the maximum band + 1 " << std::endl;
    }
    // compute the array size
    unsigned int nArrSize = (nMaxBandId > angles.size() ? nMaxBandId+1 : angles.size());
    // update the viewing mean angle
    m_sensorBandsMeanAngles.resize(nArrSize);
    for(unsigned int i = 0; i<angles.size(); i++) {
        m_sensorBandsMeanAngles[i].azimuth = angles[i].Angles.AzimuthValue;
        m_sensorBandsMeanAngles[i].zenith = angles[i].Angles.ZenithValue;
    }

    // extract the detailed viewing and solar angles
    std::vector<MACCSBandViewingAnglesGrid> maccsAngles = ComputeViewingAngles(m_metadata->ProductInformation.ViewingAngles);
    for(unsigned int i = 0; i<maccsAngles.size(); i++) {
        MACCSBandViewingAnglesGrid maccsGrid = maccsAngles[i];
        //Return only the angles of the bands for the current resolution
        if(BandAvailableForCurrentResolution(std::atoi(maccsGrid.BandId.c_str()))) {
            MetadataHelperViewingAnglesGrid mhGrid;
            mhGrid.BandId = maccsGrid.BandId;
            //mhGrid.DetectorId = maccsGrid.DetectorId;
            mhGrid.Angles.Azimuth.ColumnStep = maccsGrid.Angles.Azimuth.ColumnStep;
            mhGrid.Angles.Azimuth.ColumnUnit = maccsGrid.Angles.Azimuth.ColumnUnit;
            mhGrid.Angles.Azimuth.RowStep = maccsGrid.Angles.Azimuth.RowStep;
            mhGrid.Angles.Azimuth.RowUnit = maccsGrid.Angles.Azimuth.RowUnit;
            mhGrid.Angles.Azimuth.Values = maccsGrid.Angles.Azimuth.Values;

            mhGrid.Angles.Zenith.ColumnStep = maccsGrid.Angles.Zenith.ColumnStep;
            mhGrid.Angles.Zenith.ColumnUnit = maccsGrid.Angles.Zenith.ColumnUnit;
            mhGrid.Angles.Zenith.RowStep = maccsGrid.Angles.Zenith.RowStep;
            mhGrid.Angles.Zenith.RowUnit = maccsGrid.Angles.Zenith.RowUnit;
            mhGrid.Angles.Zenith.Values = maccsGrid.Angles.Zenith.Values;

            m_detailedViewingAngles.push_back(mhGrid);
        }
    }
    m_detailedSolarAngles.Azimuth.ColumnStep = m_metadata->ProductInformation.SolarAngles.Azimuth.ColumnStep;
    m_detailedSolarAngles.Azimuth.ColumnUnit = m_metadata->ProductInformation.SolarAngles.Azimuth.ColumnUnit;
    m_detailedSolarAngles.Azimuth.RowStep = m_metadata->ProductInformation.SolarAngles.Azimuth.RowStep;
    m_detailedSolarAngles.Azimuth.RowUnit = m_metadata->ProductInformation.SolarAngles.Azimuth.RowUnit;
    m_detailedSolarAngles.Azimuth.Values = m_metadata->ProductInformation.SolarAngles.Azimuth.Values;

    m_detailedSolarAngles.Zenith.ColumnStep = m_metadata->ProductInformation.SolarAngles.Zenith.ColumnStep;
    m_detailedSolarAngles.Zenith.ColumnUnit = m_metadata->ProductInformation.SolarAngles.Zenith.ColumnUnit;
    m_detailedSolarAngles.Zenith.RowStep = m_metadata->ProductInformation.SolarAngles.Zenith.RowStep;
    m_detailedSolarAngles.Zenith.RowUnit = m_metadata->ProductInformation.SolarAngles.Zenith.RowUnit;
    m_detailedSolarAngles.Zenith.Values = m_metadata->ProductInformation.SolarAngles.Zenith.Values;
}

bool MACCSMetadataHelper::BandAvailableForCurrentResolution(unsigned int nBand) {
    if(m_missionType == LANDSAT) {
        if(nBand < m_metadata->ProductInformation.BandWavelengths.size())
            return true;
    } else {
        // Sentinel 2
        if(nBand < m_metadata->ProductInformation.BandResolutions.size()) {
            const MACCSBandResolution& maccsBandResolution = m_metadata->ProductInformation.BandResolutions[nBand];
            int nBandRes = std::atoi(maccsBandResolution.Resolution.c_str());
            if(nBandRes == m_nResolution) {
                return true;
            }
        }
    }
    return false;
}

MetadataHelper::SingleBandShortImageType::Pointer MACCSMetadataHelper::GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult) {
    //TODO: Implement this function
}
