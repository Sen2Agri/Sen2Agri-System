#ifndef METADATAHELPER_H
#define METADATAHELPER_H

#include <string>

class MetadataHelper
{
public:
    MetadataHelper();
    virtual ~MetadataHelper();

    bool LoadMetadataFile(const std::string& file, int nResolution);

    virtual std::string GetAotImageFileName() { return m_AotFileName; }
    virtual std::string GetCloudImageFileName() { return m_CloudFileName; }
    virtual std::string GetWaterImageFileName() { return m_WaterFileName; }
    virtual std::string GetSnowImageFileName() { return m_SnowFileName; }
    virtual std::string GetImageFileName() { return m_ImageFileName; }

    virtual float GetAotQuantificationValue() { return m_fAotQuantificationValue; }
    virtual float GetAotNoDataValue() { return m_fAotNoDataVal; }
    virtual int GetAotBandIndex() { return m_nAotBandIndex; }

protected:
    virtual bool DoLoadMetadata() = 0;
    void Reset();

protected:
    std::string m_AotFileName;
    std::string m_CloudFileName;
    std::string m_WaterFileName;
    std::string m_SnowFileName;
    std::string m_ImageFileName;

    float m_fAotQuantificationValue;
    float m_fAotNoDataVal;
    float m_nAotBandIndex;

protected:
    std::string m_inputMetadataFileName;
    int m_nResolution;

};

#endif // METADATAHELPER_H
