#include "ItaCountryInfo.h"

std::string ItaCountryInfo::GetName() { return "ITA"; }
std::string ItaCountryInfo::GetUniqueId(OGRFeature &ogrFeat) {
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("parcel_id"));
}
std::string ItaCountryInfo::GetMainCrop(OGRFeature &ogrFeat) {
    if (m_practice.size() == 0) {
        // If the practice is NA, then we should not write these items
        int efaCrop = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("EFA_crop")));
        if (efaCrop != 0) {
            return "";
        }
    }
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_code"));
}
bool ItaCountryInfo::GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
    int efaCrop = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("EFA_crop")));
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
