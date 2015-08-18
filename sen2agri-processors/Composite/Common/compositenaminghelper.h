#ifndef COMPOSITENAMINGHELPER_H
#define COMPOSITENAMINGHELPER_H

#include <string>

class CompositeNamingHelper
{
public:
    CompositeNamingHelper();
    virtual ~CompositeNamingHelper();
    bool LoadMetadataFile(const std::string& file, int nRes);

    // First level of file names after preprocessing (if L8 or Spot)
    // or the original file names in case of S2, according to the given resolution
    virtual std::string GetAotFileName() { return m_AotFileName; }
    virtual std::string GetCloudFileName() { return m_CloudFileName; }
    virtual std::string GetWaterFileName() { return m_WaterFileName; }
    virtual std::string GetSnowFileName() { return m_SnowFileName; }
    virtual std::string GetImageFileName() { return m_ImageFileName; }

    virtual std::string GetPreProcessedAotFileName() { return m_PreProcessedAotFileName; }
    virtual std::string GetPreProcessedCloudFileName() { return m_PreProcessedCloudFileName; }
    virtual std::string GetPreProcessedWaterFileName() { return m_PreProcessedWaterFileName; }
    virtual std::string GetPreProcessedSnowFileName() { return m_PreProcessedSnowFileName; }
    virtual std::string GetPreProcessedImageFileName() { return m_PreProcessedImageFileName; }

    // The file name for the total weight for the given resolution
    virtual std::string GetAotWeightFileName() { return m_AotWeightFileName; }
    virtual std::string GetCloudWeightFileName() { return m_CloudWeightFileName; }
    virtual std::string GetTotalWeightFileName() { return m_TotalWeightFileName; }

protected:
    virtual bool DoLoadMetadata() = 0;
    void Reset();

protected:
    std::string m_AotFileName;
    std::string m_CloudFileName;
    std::string m_WaterFileName;
    std::string m_SnowFileName;
    std::string m_ImageFileName;

    std::string m_PreProcessedAotFileName;
    std::string m_PreProcessedCloudFileName;
    std::string m_PreProcessedWaterFileName;
    std::string m_PreProcessedSnowFileName;
    std::string m_PreProcessedImageFileName;

    std::string m_AotWeightFileName;
    std::string m_CloudWeightFileName;
    std::string m_TotalWeightFileName;

protected:
    int m_nResolution;
    std::string m_inputMetadataFileName;

};

#endif // COMPOSITENAMINGHELPER_H
