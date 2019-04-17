#include "LtuCountryInfo.h"

LtuCountryInfo::LtuCountryInfo() {
    using namespace std::placeholders;
    m_LineHandlerFnc = std::bind(&LtuCountryInfo::HandleFileLine, this, _1, _2, _3);
}

std::string LtuCountryInfo::GetName() { return "LTU"; }

void LtuCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);

    m_PSL_KODAS_FieldIdx = firstOgrFeat.GetFieldIndex("PSL_KODAS");
    m_agg_id_FieldIdx = firstOgrFeat.GetFieldIndex("agg_id");

    m_VALDOS_NR_FieldIdx = firstOgrFeat.GetFieldIndex("VALDOS_NR");
    m_KZS_NR_FieldIdx = firstOgrFeat.GetFieldIndex("KZS_NR");
    m_LAUKO_NR_FieldIdx = firstOgrFeat.GetFieldIndex("LAUKO_NR");
}

std::string LtuCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
    return GetGSAAUniqueId(ogrFeat);
}

std::string LtuCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(m_PSL_KODAS_FieldIdx);
}

bool LtuCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    const std::string &uid = GetUniqueId(ogrFeat);
    if (practice == CATCH_CROP_VAL) {
        return (m_ccISMap.find(uid) != m_ccISMap.end() ||
                m_ccPOMap.find(uid) != m_ccPOMap.end());
    } else if (practice == FALLOW_LAND_VAL) {
        return (m_blackFallowMap.find(uid) != m_blackFallowMap.end() ||
                m_greenFallowMap.find(uid) != m_greenFallowMap.end());
    } else if (practice == NITROGEN_FIXING_CROP_VAL) {
        return (m_nfcMap.find(uid) != m_nfcMap.end());
    }
    return false;
}

std::string LtuCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    const std::string &uid = GetUniqueId(ogrFeat);
    if (m_practice == CATCH_CROP_VAL) {
        if (m_ccISMap.find(uid) != m_ccISMap.end()) {
            return "IS";
        } else if (m_ccPOMap.find(uid) != m_ccPOMap.end()) {
            return "PO";
        }
    } else if (m_practice == FALLOW_LAND_VAL) {
        if (m_blackFallowMap.find(uid) != m_blackFallowMap.end()) {
            return "PDJ";
        }
        if(m_greenFallowMap.find(uid) != m_greenFallowMap.end()){
            return "PDZ";
        }
    } // else if (practice == NITROGEN_FIXING_CROP_VAL) return "NA"
    return "NA";
}

std::string LtuCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == FALLOW_LAND_VAL) {
        const std::string &pType = GetPracticeType(ogrFeat);
        if (pType == "PDZ") {
            return m_pWinterEnd;
        }
    }
    return m_pend;
}

int LtuCountryInfo::HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
    std::map<std::string, std::string> *pRefMap = NULL;
    switch(fileIdx) {
    case 0:
        if (m_practice.size() == 0  || m_practice =="NA" || m_practice == CATCH_CROP_VAL) {
            // we have a ccPO file
            pRefMap = &m_ccPOMap;
        } else if (m_practice == FALLOW_LAND_VAL) {
            pRefMap = &m_blackFallowMap;
        } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
            pRefMap = &m_nfcMap;
        }

        break;
    case 1:
        if (m_practice.size() == 0  || m_practice =="NA" || m_practice == CATCH_CROP_VAL) {
            // we have a ccIS file
            pRefMap = &m_ccISMap;
        } else if (m_practice == FALLOW_LAND_VAL) {
            pRefMap = &m_greenFallowMap;
        } else {
            std::cout << "ERROR: Unexpected file here!!!" << std::endl;
            return false;
        }
        break;
    case 2:
        // here is expected only for no practice selections
        pRefMap = &m_blackFallowMap;
        break;
    case 3:
        // here is expected only for no practice selections
        pRefMap = &m_greenFallowMap;
        break;
    case 4:
        // here is expected only for no practice selections
        pRefMap = &m_nfcMap;
        break;
    }

    if (pRefMap) {
        std::string uid;
        std::vector<std::string> keys = {"valdos numeris", "kZs", "deklaruoto lauko numeris"};
        for(size_t i = 0; i<keys.size();i++) {
            MapHdrIdx::const_iterator itMap = header.find(keys[i]);
            if (itMap != header.end() && itMap->second < line.size()) {
                if (i != 0) {
                    uid += "-";
                }
                uid += line[itMap->second];
            }
        }
        (*pRefMap)[uid] = uid;
    } else {
        std::cout << "An error occurred trying to identify the file!!!" << std::endl;
        return false;
    }
    return true;
}

bool LtuCountryInfo::HasUid(const std::string &fid, const std::map<std::string, std::string> &refMap) {
    std::map<std::string, std::string>::const_iterator itMap = refMap.find(fid);
    return itMap != refMap.end();
}

std::string LtuCountryInfo::GetGSAAUniqueId(const AttributeEntry &ogrFeat)    {
    std::string gsaaId;
    if (m_agg_id_FieldIdx >= 0) {
        gsaaId = ogrFeat.GetFieldAsString(m_agg_id_FieldIdx);
    } else {
        gsaaId = std::to_string(ogrFeat.GetFieldAsInteger(m_VALDOS_NR_FieldIdx)) + "-" +
                ogrFeat.GetFieldAsString(m_KZS_NR_FieldIdx) + "-" +
                ogrFeat.GetFieldAsString(m_LAUKO_NR_FieldIdx);
    }

    return gsaaId;
}

