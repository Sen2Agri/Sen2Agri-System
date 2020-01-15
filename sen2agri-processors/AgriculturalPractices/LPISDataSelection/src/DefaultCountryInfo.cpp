#include "DefaultCountryInfo.h"

std::string DefaultCountryInfo::GetName() { return m_strCountry; }

DefaultCountryInfo::DefaultCountryInfo() {
}

bool DefaultCountryInfo::GetHasPractice(const AttributeEntry &, const std::string &) {
    return false;
}

std::string DefaultCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return GetOriCrop(ogrFeat);
}

