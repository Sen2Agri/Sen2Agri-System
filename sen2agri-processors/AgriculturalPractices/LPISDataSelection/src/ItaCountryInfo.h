#ifndef ItaCountryInfo_H
#define ItaCountryInfo_H

#include "CountryInfoBase.h"

class ItaCountryInfo : public CountryInfoBase {
public:
    virtual std::string GetName();
    virtual std::string GetUniqueId(const AttributeEntry &ogrFeat);
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

};

#endif
