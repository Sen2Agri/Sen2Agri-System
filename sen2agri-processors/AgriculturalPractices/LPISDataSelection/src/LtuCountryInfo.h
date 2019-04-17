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
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetUniqueId(const AttributeEntry &ogrFeat);

    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);

    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);
    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);

    bool HasUid(const std::string &fid, const std::map<std::string, std::string> &refMap);

private:
    std::string GetGSAAUniqueId(const AttributeEntry &ogrFeat);

    int m_PSL_KODAS_FieldIdx;
    int m_agg_id_FieldIdx;

    int m_VALDOS_NR_FieldIdx;
    int m_KZS_NR_FieldIdx;
    int m_LAUKO_NR_FieldIdx;
};

#endif
