#ifndef ItaCountryInfo_H
#define ItaCountryInfo_H

#include "CountryInfoBase.h"

class ItaCountryInfo : public CountryInfoBase {
public:
    virtual std::string GetName();
    virtual std::string GetUniqueId(OGRFeature &ogrFeat);
    virtual std::string GetMainCrop(OGRFeature &ogrFeat);
    virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice);

};

#endif
