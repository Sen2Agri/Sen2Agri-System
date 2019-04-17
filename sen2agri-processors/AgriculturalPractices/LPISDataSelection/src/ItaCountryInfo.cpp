#include "ItaCountryInfo.h"

std::string ItaCountryInfo::GetName() { return "ITA"; }
std::string ItaCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("ori_id"));
}
std::string ItaCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    if (m_practice =="NA") {
        // If the practice is NA, then we should not write these items
        int efaCrop = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("c_efa_crop")));
        if (efaCrop != 0) {
            return "";
        }
    }
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("ori_crop"));
}
bool ItaCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    int efaCrop = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("c_efa_crop")));
    if (practice == NITROGEN_FIXING_CROP_VAL) {
        if (efaCrop != 0 && efaCrop != 214) {
            return true;
        }
    } else if (practice == FALLOW_LAND_VAL) {
        if (efaCrop == 214) {
            return true;
        }
    }
    return false;
}
