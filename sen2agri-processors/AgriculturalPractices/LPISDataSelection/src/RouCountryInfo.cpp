#include "RouCountryInfo.h"        

RouCountryInfo::RouCountryInfo() {
    using namespace std::placeholders;
    m_ShpFeatHandlerFnc = std::bind(&RouCountryInfo::HandleCCFeature, this, _1, _2);
}

std::string RouCountryInfo::GetName() { return "ROU"; }

std::string RouCountryInfo::GetUniqueId(OGRFeature &ogrFeat) {
    return GetGSAAUniqueId(ogrFeat);
}

std::string RouCountryInfo::GetMainCrop(OGRFeature &ogrFeat) {
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_code"));
}

bool RouCountryInfo::GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
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

std::string RouCountryInfo::GetPEnd(OGRFeature &ogrFeat) {
    if (m_practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        switch(cropCode) {
            case 1511:
            case 15171:
            case 1591:
                return "2018-05-27";
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
                return "2018-04-29";
        }
        return m_pend;
    } else if (m_practice == NITROGEN_FIXING_CROP_VAL || m_practice == FALLOW_LAND_VAL) {
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

std::string RouCountryInfo::GetGSAAUniqueId(OGRFeature &ogrFeat) {
    int aggId = ogrFeat.GetFieldIndex("agg_id");
    std::string gsaaId;
    if (aggId >= 0) {
        gsaaId = ogrFeat.GetFieldAsString(aggId);
    } else {
        const std::string &idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("ID_unic")));
        const std::string &parcelNo = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("parcel_nr")));
        gsaaId = idUnic + "-" + parcelNo + "-" +
                ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_nr"));
    }

    return gsaaId;
}

