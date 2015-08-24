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

    void UpdateValuesForLandsat();
    void UpdateValuesForSentinel();

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();

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

    //virtual std::unique_ptr<itk::LightObject> GetMetadata() { return m_metadata; }

protected:
    typedef enum {S2, LANDSAT} MissionType;
    MissionType m_missionType;
    std::unique_ptr<MACCSFileMetadata> m_metadata;
};

#endif // S2METADATAHELPER_H
