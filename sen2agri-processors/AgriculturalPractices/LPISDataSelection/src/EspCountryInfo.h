#ifndef EspCountryInfo_H
#define EspCountryInfo_H

#include "CountryInfoBase.h"

class EspCountryInfo : public CountryInfoBase {
private:
    const std::map<int, int> m_nflCropCodes = {{34,   34}, {40,   40}, {41,   41}, {43,   43}, {49,   49}, {50,   50}, {51,   51}, {52,   52}, {53,   53}, {60,   60},
{61,   61}, {67,   67}, {76,   76}, {87,   87}, {180,  180}, {238,  238}, {239,  239}, {240,  240}, {248,  248}, {249,  249},
{250,  250}, {77,   77 }, {241,  241}, {242,  242}, {243,  243}, {244,  244}, {245,  245}, {246,  246}, {298,  298}, {299,  299},
{336,  336}, {337,  337}, {338,  338}, {339,  339}, {340,  340}, {342,   342}};
public:
    virtual std::string GetName();

    virtual std::string GetUniqueId(const AttributeEntry &ogrFeat);
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    std::string RemoveSuffix(const std::string &field);
};

#endif
