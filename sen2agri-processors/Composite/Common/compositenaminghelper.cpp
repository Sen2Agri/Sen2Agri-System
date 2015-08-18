#include "compositenaminghelper.h"

CompositeNamingHelper::CompositeNamingHelper()
{
}

CompositeNamingHelper::~CompositeNamingHelper()
{
}

bool CompositeNamingHelper::LoadMetadataFile(const std::string& file, int nRes)
{
    Reset();
    m_nResolution = nRes;
    m_inputMetadataFileName = file;
    return DoLoadMetadata();
}

void CompositeNamingHelper::Reset()
{
    m_AotFileName = "";
    m_CloudFileName = "";
    m_WaterFileName = "";
    m_SnowFileName = "";
    m_ImageFileName = "";

    m_PreProcessedAotFileName = "";
    m_PreProcessedCloudFileName = "";
    m_PreProcessedWaterFileName = "";
    m_PreProcessedSnowFileName = "";
    m_PreProcessedImageFileName = "";

    m_AotWeightFileName = "";
    m_CloudWeightFileName = "";
    m_TotalWeightFileName = "";
}
