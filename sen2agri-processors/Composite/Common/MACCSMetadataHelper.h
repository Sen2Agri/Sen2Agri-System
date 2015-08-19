#ifndef S2METADATAHELPER_H
#define S2METADATAHELPER_H

#include "MetadataHelper.h"
#include <vector>

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;

class MACCSMetadataHelper : public MetadataHelper
{
public:
    MACCSMetadataHelper();

    const char * GetNameOfClass() { return "S2MetadataHelper"; }

protected:
    virtual bool DoLoadMetadata();

    void UpdateValuesForLandsat(const MACCSFileMetadata &meta);
    void UpdateValuesForSentinel(const MACCSFileMetadata &meta);

    std::string DeriveFileNameFromImageFileName(const MACCSFileMetadata& meta, const std::string& replacement);
    std::string buildFullPath(const std::string& fileName);

    std::string getImageFileName(const MACCSFileMetadata& meta);
    std::string getAotFileName(const MACCSFileMetadata& meta);
    std::string getCloudFileName(const MACCSFileMetadata& meta);
    std::string getWaterFileName(const MACCSFileMetadata& meta);
    std::string getSnowFileName(const MACCSFileMetadata& meta);

    std::string getMACCSImageFileName(const std::string& descriptor,
                                      const std::vector<MACCSFileInformation>& imageFiles,
                                      const std::string& ending);
    std::string getMACCSImageFileName(const std::string& descriptor,
                                      const std::vector<MACCSAnnexInformation>& maskFiles,
                                      const std::string& ending);
    std::string getMACCSImageHdrName(const std::string& descriptor,
                                     const std::vector<MACCSAnnexInformation>& maskFiles,
                                     const std::string& ending);

    void ReadSpecificMACCSHdrFile(const std::string& fileName);
    int getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name);

protected:
    typedef enum {S2, LANDSAT} MissionType;
    MissionType m_missionType;
};

#endif // S2METADATAHELPER_H
