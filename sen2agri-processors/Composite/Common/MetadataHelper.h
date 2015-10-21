#ifndef METADATAHELPER_H
#define METADATAHELPER_H

#include <string>
#include <vector>

#define UNUSED(expr) do { (void)(expr); } while (0)

#define LANDSAT_MISSION_STR    "LANDSAT"
#define SENTINEL_MISSION_STR   "SENTINEL"
#define SPOT4_MISSION_STR      "SPOT4"

typedef struct {
    double zenith;
    double azimuth;
} MeanAngles_Type;

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
    virtual int GetBlueBandIndex() { return m_nBlueBandIndex; }
    virtual int GetGreenBandIndex() { return m_nGreenBandIndex; }
    virtual int GetNirBandIndex() { return m_nNirBandIndex; }

    // angles
    virtual bool HasGlobalMeanAngles() { return m_bHasGlobalMeanAngles; }
    virtual bool HasBandMeanAngles() { return m_bHasBandMeanAngles; }
    virtual MeanAngles_Type GetSolarMeanAngles() { return m_solarMeanAngles;}
    virtual MeanAngles_Type GetSensorMeanAngles();
    virtual double GetRelativeAzimuthAngle();
    virtual MeanAngles_Type GetSensorMeanAngles(int nBand);

    virtual int GetTotalBandsNo() { return m_nTotalBandsNo; }
    virtual int GetBandsNoForCurrentResolution() { return m_nBandsNoForCurRes; }
    virtual std::string GetBandName(unsigned int nRelBandIdx, bool bRelativeIdx=true) = 0;
    // In the case of multiple resolutions in multiple files, we need to know the
    // index of an absolute index band in its file
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx) = 0;
    virtual bool GetTrueColourBandIndexes(int &redIdx, int &greenIdx, int &blueIdx);

protected:
    virtual bool DoLoadMetadata() = 0;
    void Reset();
    std::string extractFolder(const std::string& filename);
    std::string buildFullPath(const std::string& fileName);

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
    int m_nBlueBandIndex;
    int m_nGreenBandIndex;
    int m_nNirBandIndex;
    int m_nTotalBandsNo;
    unsigned int m_nBandsNoForCurRes;

    MeanAngles_Type m_solarMeanAngles;
    std::vector<MeanAngles_Type> m_sensorBandsMeanAngles;
    bool m_bHasGlobalMeanAngles;
    bool m_bHasBandMeanAngles;

protected:
    std::string m_inputMetadataFileName;
    int m_nResolution;
    std::string m_DirName;
};

#endif // METADATAHELPER_H
