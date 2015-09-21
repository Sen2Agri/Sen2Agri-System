#include "MetadataHelper.h"
#include <time.h>
#include <ctime>
#include <cmath>

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

    m_nRedBandIndex = -1;
    m_nGreenBandIndex = -1;
    m_nNirBandIndex = -1;
}

int MetadataHelper::GetAcquisitionDateAsDoy()
{
    struct tm tmDate = {};
    if (strptime(m_AcquisitionDate.c_str(), "%Y%m%d", &tmDate) == NULL) {
        return -1;
    }
    auto curTime = std::mktime(&tmDate);

    std::tm tmYearStart = {};
    tmYearStart.tm_year = tmDate.tm_year;
    tmYearStart.tm_mon = 0;
    tmYearStart.tm_mday = 1;

    auto yearStart = std::mktime(&tmYearStart);
    auto diff = curTime - yearStart;

    return lrintf(diff / 86400 /* 60*60*24*/);
}
