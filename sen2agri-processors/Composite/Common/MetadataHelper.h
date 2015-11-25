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

struct MetadataHelperAngleList
{
    std::string ColumnUnit;
    std::string ColumnStep;
    std::string RowUnit;
    std::string RowStep;
    std::vector<std::vector<double> > Values;
};

struct MetadataHelperAngles
{
    MetadataHelperAngleList Zenith;
    MetadataHelperAngleList Azimuth;
};

struct MetadataHelperViewingAnglesGrid
{
    std::string BandId;
    std::string DetectorId;
    MetadataHelperAngles Angles;
};

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

    virtual int GetAbsRedBandIndex() { return m_nAbsRedBandIndex; }
    virtual int GetAbsBlueBandIndex() { return m_nAbsBlueBandIndex; }
    virtual int GetAbsGreenBandIndex() { return m_nAbsGreenBandIndex; }
    virtual int GetAbsNirBandIndex() { return m_nAbsNirBandIndex; }

    virtual int GetRelRedBandIndex() { return m_nRelRedBandIndex; }
    virtual int GetRelBlueBandIndex() { return m_nRelBlueBandIndex; }
    virtual int GetRelGreenBandIndex() { return m_nRelGreenBandIndex; }
    virtual int GetRelNirBandIndex() { return m_nRelNirBandIndex; }

    // angles
    virtual bool HasGlobalMeanAngles() { return m_bHasGlobalMeanAngles; }
    virtual bool HasBandMeanAngles() { return m_bHasBandMeanAngles; }
    virtual MeanAngles_Type GetSolarMeanAngles() { return m_solarMeanAngles;}
    virtual MeanAngles_Type GetSensorMeanAngles();
    virtual double GetRelativeAzimuthAngle();
    virtual MeanAngles_Type GetSensorMeanAngles(int nBand);

    virtual bool HasDetailedAngles() { return m_bHasDetailedAngles; }
    virtual int GetDetailedAnglesGridSize() { return m_detailedAnglesGridSize; }
    virtual MetadataHelperAngles GetDetailedSolarAngles() { return m_detailedSolarAngles; }
    virtual std::vector<MetadataHelperViewingAnglesGrid> GetDetailedViewingAngles() { return m_detailedViewingAngles; }

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

    int m_nAbsRedBandIndex;
    int m_nAbsBlueBandIndex;
    int m_nAbsGreenBandIndex;
    int m_nAbsNirBandIndex;

    int m_nRelRedBandIndex;
    int m_nRelBlueBandIndex;
    int m_nRelGreenBandIndex;
    int m_nRelNirBandIndex;

    int m_nTotalBandsNo;
    unsigned int m_nBandsNoForCurRes;

    MeanAngles_Type m_solarMeanAngles;
    std::vector<MeanAngles_Type> m_sensorBandsMeanAngles;
    bool m_bHasGlobalMeanAngles;
    bool m_bHasBandMeanAngles;

    bool m_bHasDetailedAngles;
    int m_detailedAnglesGridSize;
    MetadataHelperAngles m_detailedSolarAngles;
    std::vector<MetadataHelperViewingAnglesGrid> m_detailedViewingAngles;

protected:
    std::string m_inputMetadataFileName;
    int m_nResolution;
    std::string m_DirName;
};

#endif // METADATAHELPER_H
