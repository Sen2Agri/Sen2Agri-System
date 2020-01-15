#ifndef DefaultCountryInfo_H
#define DefaultCountryInfo_H

#include "CountryInfoBase.h"

class DefaultCountryInfo  : public CountryInfoBase {
public:
    DefaultCountryInfo();
    virtual std::string GetName();
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);
    std::string GetMainCrop(const AttributeEntry &ogrFeat);

private:
    std::string m_strCountry;
};

#endif
