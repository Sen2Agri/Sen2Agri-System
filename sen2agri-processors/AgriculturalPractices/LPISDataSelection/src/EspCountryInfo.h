#ifndef EspCountryInfo_H
#define EspCountryInfo_H

#include "CountryInfoBase.h"

class EspCountryInfo : public CountryInfoBase {
private:
    std::map<int, int> m_fallowCropCodes;
    std::map<int, int> m_naCropCodes;
    std::map<int, int> m_nfcCropCodes;
public:
    EspCountryInfo();
    virtual std::string GetName();
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetOriId(const AttributeEntry &ogrFeat);
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);
    virtual bool IsMonitoringParcel(const AttributeEntry &ogrFeat);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    std::string RemoveSuffix(const std::string &field);

private:
    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int);
    int m_cDeclaraFieldIdx;
    int m_cProductoFieldIdx;
    int m_cVariedadFieldIdx;
    int m_ctNumFieldIdx;
};

#endif
