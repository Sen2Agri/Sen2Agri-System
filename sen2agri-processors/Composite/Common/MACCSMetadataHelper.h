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

    virtual std::string GetBandName(unsigned int nBandIdx, bool bRelativeIdx=true);
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx);
    virtual float GetAotQuantificationValue();
    virtual float GetAotNoDataValue();
    virtual int GetAotBandIndex();

protected:
    virtual bool DoLoadMetadata();

    void UpdateValuesForLandsat();
    void UpdateValuesForSentinel();

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();

    std::string getMACCSImageFileName(const std::vector<MACCSFileInformation>& imageFiles,
                                      const std::string& ending);
    std::string getMACCSImageFileName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                      const std::string& ending);
    std::string getMACCSImageHdrName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                     const std::string& ending);
    std::string getMACCSImageHdrName(const std::vector<MACCSFileInformation>& imageFiles,
                                                          const std::string& ending);
    void ReadSpecificMACCSImgHdrFile();
    void ReadSpecificMACCSAotHdrFile();
    void ReadSpecificMACCSCldHdrFile();
    void ReadSpecificMACCSMskHdrFile();
    int getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name);

    void InitializeS2Angles();
    bool BandAvailableForCurrentResolution(unsigned int nBand);
    const MACCSResolution& GetMACCSResolutionInfo(int nResolution);
    std::vector<MACCSBand> GetAllMACCSBandsInfos();
    //virtual std::unique_ptr<itk::LightObject> GetMetadata() { return m_metadata; }

    virtual MetadataHelper::SingleBandShortImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult);

protected:
    typedef enum {S2, LANDSAT} MissionType;
    MissionType m_missionType;
    std::unique_ptr<MACCSFileMetadata> m_metadata;
    std::unique_ptr<MACCSFileMetadata> m_specificAotMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificImgMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificCldMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificMskMetadata;
};

#endif // S2METADATAHELPER_H
