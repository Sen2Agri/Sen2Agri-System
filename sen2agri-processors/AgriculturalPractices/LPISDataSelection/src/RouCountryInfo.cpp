#include "RouCountryInfo.h"        

RouCountryInfo::RouCountryInfo() {
    using namespace std::placeholders;
    m_ShpFeatHandlerFnc = std::bind(&RouCountryInfo::HandleCCFeature, this, _1, _2);
}

std::string RouCountryInfo::GetName() { return "ROU"; }

std::string RouCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
    return GetGSAAUniqueId(ogrFeat);
}

std::string RouCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_code"));
}

bool RouCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (practice == CATCH_CROP_VAL) {
        const std::string &uid = GetUniqueId(ogrFeat);
        if (m_gsaaIdsMap.find(uid) != m_gsaaIdsMap.end()) {
            return true;
        }
    } else if (practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        if (m_nfcCropCodes.find(cropCode) != m_nfcCropCodes.end()) {
            return true;
        }
    }
    return false;
}

std::string RouCountryInfo::GetPStart(const AttributeEntry &ogrFeat) {
    if (m_practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        switch(cropCode) {
            case 9741:
            case 95561:
            case 97471:
            case 97481:
                return "2018-03-19";
        }
    }
    return m_pstart;
}

std::string RouCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        switch(cropCode) {
            case 1511:
            case 15171:
            case 1591:
                return "2018-06-03";
            case 1281:
            case 1291:
            case 1301:
                return "2018-08-26";
            case 9731:
            case 95531:
            case 95541:
            case 9751:
                return "2018-06-30";
            case 9741:
            case 95561:
            case 97471:
            case 97481:
                return "2018-05-06";
        }
        return m_pend;
    } else if (m_practice == FALLOW_LAND_VAL) {
        return m_pend;
    }
    return "NA";
}

int RouCountryInfo::HandleCCFeature(OGRFeature &ogrFeat, int fileIdx) {
    if (fileIdx > 0) {
        std::cout << "The Romania country supports only one shp file as input" << std::endl;
        return false;
    }
    const std::string &gsaaId = GetGSAAUniqueId(ogrFeat);
    m_gsaaIdsMap[gsaaId] = 1;
    return true;
}

template <typename T>
std::string RouCountryInfo::GetGSAAUniqueId(T &ogrFeat) {
    std::string idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("ID_unic")));
    if (idUnic.size() == 0 || idUnic == "0") {
        idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("id_unic")));
    }
    const std::string &parcelNo = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("parcel_nr")));
    std::string gsaaId = idUnic + "-" + parcelNo + "-" +
            ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_nr"));

    return gsaaId;
}

