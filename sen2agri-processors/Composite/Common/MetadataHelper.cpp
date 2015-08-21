#include "MetadataHelper.h"
#include <time.h>

MetadataHelper::MetadataHelper()
{
    m_nResolution = -1;
}

MetadataHelper::~MetadataHelper()
{

}

bool MetadataHelper::LoadMetadataFile(const std::string& file, int nResolution)
{
    Reset();
    m_inputMetadataFileName = file;
    m_nResolution = nResolution;
    return DoLoadMetadata();
}

void MetadataHelper::Reset()
{
    m_Mission = "";

    m_AotFileName = "";
    m_CloudFileName = "";
    m_WaterFileName = "";
    m_SnowFileName = "";
    m_ImageFileName = "";
    m_AcquisitionDate = "";

    m_ReflQuantifVal = 1;

    m_fAotQuantificationValue = 0.0;
    m_fAotNoDataVal = 0;
    m_nAotBandIndex = -1;
}

int MetadataHelper::GetAcquisitionDateInDays()
{
    struct tm tmTime = {};
    strptime(m_AcquisitionDate.c_str(), "%Y%m%d", &tmTime);
    time_t ttTime = mktime(&tmTime);
    return ttTime/(3600*24);
}
