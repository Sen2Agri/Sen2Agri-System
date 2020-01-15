#ifndef FraCountryInfo_H
#define FraCountryInfo_H

#include "CountryInfoBase.h"

class FraCountryInfo  : public CountryInfoBase {
public:
    FraCountryInfo();
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetName();
    std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);
    virtual std::string GetPractice(const AttributeEntry &ogrFeat);
    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);
    virtual void SetPStart(const std::string &val);

    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int);

private:
    int m_CULTURE_D1_FieldIdx;
    int m_CULTURE_D2_FieldIdx;
    int m_PACAGE_FieldIdx;


    std::map<std::string, std::string> m_mapPstart;
};

#endif
