#include "FraCountryInfo.h"
#include "CommonFunctions.h"

#define SEC_IN_8_WEEKS 4838400


std::string FraCountryInfo::GetName() { return "FRA"; }

FraCountryInfo::FraCountryInfo() {
}

void FraCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);

    m_CULTURE_D1_FieldIdx = firstOgrFeat.GetFieldIndex("CULTURE_D1");
    m_CULTURE_D2_FieldIdx = firstOgrFeat.GetFieldIndex("CULTURE_D2");
    m_PACAGE_FieldIdx = firstOgrFeat.GetFieldIndex("PACAGE");
}

std::string FraCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return GetOriCrop(ogrFeat);
}

bool FraCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (practice == CATCH_CROP_VAL) {
        std::string field1 = ogrFeat.GetFieldAsString(m_CULTURE_D1_FieldIdx);
        std::string field2 = ogrFeat.GetFieldAsString(m_CULTURE_D2_FieldIdx);
        if (field1.size() > 0 || field2.size()) {
            return true;
        }
    }

    return false;
}

std::string FraCountryInfo::GetPractice(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        std::string field1 = ogrFeat.GetFieldAsString(m_CULTURE_D1_FieldIdx);
        std::string field2 = ogrFeat.GetFieldAsString(m_CULTURE_D2_FieldIdx);
        if (field1.size() > 0 || field2.size()) {
            return m_practice;
        }
        return "NA";
    }
    // NO NFC or Fallow for FRA

    return m_practice;
}

std::string FraCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    std::string retPtype = "NA";
    if (m_practice == CATCH_CROP_VAL) {
        std::string field1 = ogrFeat.GetFieldAsString(m_CULTURE_D1_FieldIdx);
        std::string field2 = ogrFeat.GetFieldAsString(m_CULTURE_D2_FieldIdx);
        if (field1.size() > 0 && field2.size() > 0) {
            retPtype = "CatchCrop_1";
        } else if (field1.size() > 0 || field2.size() > 0) {
            retPtype = "CatchCrop_2";
        }
    }
    return retPtype;
}

void FraCountryInfo::SetPStart(const std::string &val) {
    if (m_practice == CATCH_CROP_VAL) {
        std::vector<std::string> pairs;
        boost::split(pairs, val, boost::is_any_of(","));
        for (const std::string &pair: pairs) {
            std::vector<std::string> vals;
            boost::split(vals, pair, boost::is_any_of(":"));
            if (vals.size() == 2 && vals[0].length() > 0 && vals[1].length() > 0) {
                m_mapPstart[vals[0]] = vals[1];
            }
        }
    } else {
        m_pstart = val;
    }
}

std::string FraCountryInfo::GetPStart(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        std::string pacage = ogrFeat.GetFieldAsString(m_PACAGE_FieldIdx);
        if (pacage.length() > 3) {
            const std::string &depIdStr = pacage.substr(1, 2);
            std::map<std::string, std::string>::const_iterator it = m_mapPstart.find(depIdStr);
            if (it != m_mapPstart.end()) {
                return it->second;
            }
        }
    }
    return "NA";
}
std::string FraCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    const std::string &pStartStr = GetPStart(ogrFeat);
    if (pStartStr != "NA") {
        time_t ttTime = GetTimeFromString(pStartStr);
        ttTime += SEC_IN_8_WEEKS;
        return TimeToString(ttTime);
    }
    return "NA";

}
