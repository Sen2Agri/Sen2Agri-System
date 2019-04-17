#include "EspCountryInfo.h"

std::string EspCountryInfo::GetName() { return "ESP"; }

std::string EspCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_DECLARA"));
}
std::string EspCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return RemoveSuffix(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO")));
}
bool EspCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (practice == FALLOW_LAND_VAL) {
        int cropCode = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO")));
        int variedad = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_VARIEDAD")));
        if (cropCode == 20 || cropCode == 23) {
            if (variedad == 901 || variedad == 902) {
                return true;
            }
        } else if (cropCode == 21 || cropCode == 25 || cropCode == 334) {
            if (variedad == 0) {
                return true;
            }
        }
    } else if (practice == NITROGEN_FIXING_CROP_VAL) {
         int cropCode = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO")));
         if (m_nflCropCodes.find(cropCode) != m_nflCropCodes.end()) {
             return true;
         }
    }
    return false;
}

std::string EspCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    if (m_practice == FALLOW_LAND_VAL) {
        return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_VARIEDAD"));
    } // else if (practice == NITROGEN_FIXING_CROP_VAL) return "NA"
    return "NA";
}
std::string EspCountryInfo::RemoveSuffix(const std::string &field) {
    size_t lastindex = field.find_last_of(".");
    if (lastindex != field.npos) {
        return field.substr(0, lastindex);
    }
    return field;
}

