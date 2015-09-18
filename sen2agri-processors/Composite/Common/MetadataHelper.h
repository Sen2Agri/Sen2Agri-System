#ifndef METADATAHELPER_H
#define METADATAHELPER_H

#include <string>

#define LANDSAT_MISSION_STR    "LANDSAT"
#define SENTINEL_MISSION_STR   "SENTINEL"
#define SPOT4_MISSION_STR      "SPOT4"


class MetadataHelper
{
public:
    MetadataHelper();
    virtual ~MetadataHelper();

    bool LoadMetadataFile(const std::string& file, int nResolution = -1);

    virtual std::string GetMissionName() { return m_Mission; }

    virtual std::string GetAotImageFileName() { return m_AotFileName; }
    virtual std::string GetCloudImageFileName() { return m_CloudFileName; }
    virtual std::string GetWaterImageFileName() { return m_WaterFileName; }
    virtual std::string GetSnowImageFileName() { return m_SnowFileName; }
    virtual std::string GetImageFileName() { return m_ImageFileName; }

    // returns the acquisition date in the format YYYYMMDD
    virtual std::string GetAcquisitionDate() { return m_AcquisitionDate; }
    virtual int GetAcquisitionDateAsDoy();

    virtual double GetReflectanceQuantificationValue() {return m_ReflQuantifVal; }
    virtual float GetAotQuantificationValue() { return m_fAotQuantificationValue; }
    virtual float GetAotNoDataValue() { return m_fAotNoDataVal; }
    virtual int GetAotBandIndex() { return m_nAotBandIndex; }

    virtual int GetRedBandIndex() { return m_nRedBandIndex; }
    virtual int GetGreenBandIndex() { return m_nGreenBandIndex; }
    virtual int GetNirBandIndex() { return m_nNirBandIndex; }

protected:
    virtual bool DoLoadMetadata() = 0;
    void Reset();

protected:
    std::string m_Mission;

    std::string m_AotFileName;
    std::string m_CloudFileName;
    std::string m_WaterFileName;
    std::string m_SnowFileName;
    std::string m_ImageFileName;
    std::string m_AcquisitionDate;

    double m_ReflQuantifVal;

    float m_fAotQuantificationValue;
    float m_fAotNoDataVal;
    int m_nAotBandIndex;

    int m_nRedBandIndex;
    int m_nGreenBandIndex;
    int m_nNirBandIndex;

protected:
    std::string m_inputMetadataFileName;
    int m_nResolution;

};

#endif // METADATAHELPER_H
