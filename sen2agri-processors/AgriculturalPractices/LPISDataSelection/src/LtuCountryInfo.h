#ifndef LtuCountryInfo_H
#define LtuCountryInfo_H

#include "CountryInfoBase.h"

class LtuCountryInfo : public CountryInfoBase {
private :
    std::map<std::string, std::string> m_ccISMap;
    std::map<std::string, std::string> m_ccPOMap;
    std::map<std::string, std::string> m_greenFallowMap;
    std::map<std::string, std::string> m_blackFallowMap;
    std::map<std::string, std::string> m_nfcMap;

public:
    LtuCountryInfo();

    virtual std::string GetName();
    virtual std::string GetUniqueId(OGRFeature &ogrFeat);

    virtual std::string GetMainCrop(OGRFeature &ogrFeat);
    virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(OGRFeature &ogrFeat);

    virtual std::string GetPEnd(OGRFeature &ogrFeat);
    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);

    bool HasUid(const std::string &fid, const std::map<std::string, std::string> &refMap);

private:
    std::string GetGSAAUniqueId(OGRFeature &ogrFeat);
};

#endif
