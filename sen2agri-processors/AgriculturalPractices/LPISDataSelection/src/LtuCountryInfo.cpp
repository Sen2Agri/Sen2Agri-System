#include "LtuCountryInfo.h"
#include "CommonFunctions.h"

LtuCountryInfo::LtuCountryInfo() : m_is2019FileFormat(-1), m_discrimCsvColIdx(-1), m_additionalDiscrimCsvColIdx(-1) {
    m_2018FileCsvKeys = {"valdos numeris", "kZs", "deklaruoto lauko numeris"};
    std::string kzsNr("K");
    kzsNr.append<int>(1,0x8E);
    kzsNr.append("S Nr.");
    m_2019FileCsvKeys = {"Valda", kzsNr, "Lauko nr."};
    m_2019CCFileCsvKeys = {"Holding Nr.", kzsNr, "Parcel Nr."};
    m_DiscrimCsvColName = "Kodas";
    m_AdditionalDiscrimCsvColName = "Pas?lis";
    m_DiscrimISCsvVal = "EASV_ZOL_ISELIO_ZI";
    m_DiscrimPOCsvVal = "EASV_ZIEMINIS_POS";
    m_DiscrimSPCCCsvVal = "EASV_POSELIS_PS";

    m_DiscrimFLCsvVal = "EASV_PUDYMO_PD";
    m_DiscrimAddPDZFLCsvVal = "PD";
    m_DiscrimAddPDZFLCsvVal.append<int>(1,0x8E);
    m_DiscrimAddPDJFLCsvVal = "PDJ";

    m_DiscrimNFCCsvVal = "EASV_AZOT_AZ";

    using namespace std::placeholders;
    m_LineHandlerFnc = std::bind(&LtuCountryInfo::HandleFileLine, this, _1, _2, _3);
}

std::string LtuCountryInfo::GetName() { return "LTU"; }

void LtuCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);

    m_PSL_KODAS_FieldIdx = firstOgrFeat.GetFieldIndex("PSL_KODAS");

    m_VALDOS_NR_FieldIdx = firstOgrFeat.GetFieldIndex("VALDOS_NR");
    m_KZS_NR_FieldIdx = firstOgrFeat.GetFieldIndex("KZS_NR");
    m_LAUKO_NR_FieldIdx = firstOgrFeat.GetFieldIndex("LAUKO_NR");
}

std::string LtuCountryInfo::GetOriId(const AttributeEntry &ogrFeat) {
    return GetGSAAUniqueId(ogrFeat);
}

std::string LtuCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(m_PSL_KODAS_FieldIdx);
}

bool LtuCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    const std::string &uid = GetOriId(ogrFeat);
    if (practice == CATCH_CROP_VAL) {
        return (m_ccISMap.find(uid) != m_ccISMap.end() ||
                m_ccPOMap.find(uid) != m_ccPOMap.end() ||
                m_ccSPMap.find(uid) != m_ccSPMap.end());
    } else if (practice == FALLOW_LAND_VAL) {
        return (m_blackFallowMap.find(uid) != m_blackFallowMap.end() ||
                m_greenFallowMap.find(uid) != m_greenFallowMap.end());
    } else if (practice == NITROGEN_FIXING_CROP_VAL) {
        return (m_nfcMap.find(uid) != m_nfcMap.end());
    }
    return false;
}

std::string LtuCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    const std::string &uid = GetOriId(ogrFeat);
    if (m_practice == CATCH_CROP_VAL) {
        if (m_ccISMap.find(uid) != m_ccISMap.end()) {
            return "IS";
        } else if (m_ccPOMap.find(uid) != m_ccPOMap.end()) {
            if (m_is2019FileFormat) {
                return "WP";
            }
            return "PO";
        } else if (m_ccSPMap.find(uid) != m_ccSPMap.end()) {
            return "SP";    // summer plant
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

std::string LtuCountryInfo::GetPStart(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        const std::string &uid = GetOriId(ogrFeat);
        if (m_ccISMap.find(uid) != m_ccISMap.end()) {
            std::map<std::string, CCPracticeDatesInfos>::const_iterator itMap = m_ccISPracticeDatesFilterMap.find(uid);
            if (itMap != m_ccISPracticeDatesFilterMap.end()) {
                return itMap->second.pStart;
            }
        }
        if (m_ccSPMap.find(uid) != m_ccSPMap.end()) {
            std::map<std::string, CCPracticeDatesInfos>::const_iterator itMap = m_ccSPPracticeDatesFilterMap.find(uid);
            if (itMap != m_ccSPPracticeDatesFilterMap.end()) {
                return itMap->second.pStart;
            }
        }

    }
    return m_pstart;
}

std::string LtuCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == FALLOW_LAND_VAL) {
        const std::string &pType = GetPracticeType(ogrFeat);
        if (pType == "PDZ") {
            return m_pWinterEnd;
        }
    } else if (m_practice == CATCH_CROP_VAL) {
        const std::string &uid = GetOriId(ogrFeat);
        if (m_ccISMap.find(uid) != m_ccISMap.end()) {
            std::map<std::string, CCPracticeDatesInfos>::const_iterator itMap = m_ccISPracticeDatesFilterMap.find(uid);
            if (itMap != m_ccISPracticeDatesFilterMap.end()) {
                return itMap->second.pEnd;
            }
        }
        if (m_ccSPMap.find(uid) != m_ccSPMap.end()) {
            std::map<std::string, CCPracticeDatesInfos>::const_iterator itMap = m_ccSPPracticeDatesFilterMap.find(uid);
            if (itMap != m_ccSPPracticeDatesFilterMap.end()) {
                return itMap->second.pEnd;
            }
        }
    }
    return m_pend;
}

std::string LtuCountryInfo::GetHStart(const AttributeEntry &ogrFeat) {
    if (m_practice == FALLOW_LAND_VAL) {
        const std::string &pType = GetPracticeType(ogrFeat);
        if (pType == "PDZ") {
            return m_hWinterStart;
        }
    }

    return m_hstart;
}


int LtuCountryInfo::HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
    if (Is2019FileFormat(header)) {
        return Handle2019FileLine(header, line, fileIdx);
    }
    return Handle2018FileLine(header, line, fileIdx);
}

int LtuCountryInfo::Handle2019FileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
    switch(fileIdx) {
        case 0:
        {
            // we know here that we have the updated index for the discrimination column
            std::map<std::string, std::string> *pRefMap = NULL;
            const std::string &strToCheck = line[m_discrimCsvColIdx];
            if (strToCheck == m_DiscrimISCsvVal) {
                pRefMap = &m_ccISMap;
            } else if (strToCheck == m_DiscrimPOCsvVal) {
                pRefMap = &m_ccPOMap;
            } else if (strToCheck == m_DiscrimSPCCCsvVal) {
                pRefMap = &m_ccSPMap;
            } else if (strToCheck == m_DiscrimFLCsvVal) {
                pRefMap = &m_greenFallowMap;
                if (m_additionalDiscrimCsvColIdx != -1) {
                    const std::string &addStrToCheck = line[m_additionalDiscrimCsvColIdx];
                    if (addStrToCheck == m_DiscrimAddPDZFLCsvVal) {
                        pRefMap = &m_greenFallowMap;
                    } else if (addStrToCheck == m_DiscrimAddPDJFLCsvVal) {
                        pRefMap = &m_blackFallowMap;
                    }
                }
            } else if (strToCheck == m_DiscrimNFCCsvVal) {
                pRefMap = &m_nfcMap;
            }
            return AddUuidToMap(header, m_2019FileCsvKeys, line, pRefMap);
        }
        case 1:
            // load the catch crop file containing parcels that have different P_START and P_END dates
            return Handle2019CatchCropFileLine(header, line, m_ccISPracticeDatesFilterMap);
        case 2:
            // load the catch crop file containing parcels that have different P_START and P_END dates
            return Handle2019CatchCropFileLine(header, line, m_ccSPPracticeDatesFilterMap);
        default:
            return false;
    }
}

int LtuCountryInfo::Handle2018FileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
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

    return AddUuidToMap(header, m_2018FileCsvKeys, line, pRefMap);
}

bool LtuCountryInfo::AddUuidToMap(const MapHdrIdx& header, const std::vector<std::string> &keys, const std::vector<std::string> &line,
                  std::map<std::string, std::string> *pRefMap) {
    if (pRefMap) {
        std::string uid;
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
        if (!m_is2019FileFormat) {
            std::cout << "An error occurred trying to identify the file!!!" << std::endl;
        }
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
    if (m_OrigIdFieldIdx >= 0) {
        gsaaId = ogrFeat.GetFieldAsString(m_OrigIdFieldIdx);
    } else {
        gsaaId = std::to_string(ogrFeat.GetFieldAsInteger(m_VALDOS_NR_FieldIdx)) + "-" +
                ogrFeat.GetFieldAsString(m_KZS_NR_FieldIdx) + "-" +
                ogrFeat.GetFieldAsString(m_LAUKO_NR_FieldIdx);
    }

    return gsaaId;
}

bool LtuCountryInfo::Is2019FileFormat(const MapHdrIdx& header) {
    if (m_is2019FileFormat != -1) {
        return m_is2019FileFormat;
    }

    MapHdrIdx::const_iterator itMap = header.find(m_DiscrimCsvColName);
    if (itMap != header.end()) {
        if (CheckColumnsInHeader(header, m_2019FileCsvKeys)) {
            m_discrimCsvColIdx = itMap->second;
            m_is2019FileFormat = 1;
            itMap = header.find(m_AdditionalDiscrimCsvColName);
            if (itMap != header.end()) {
                m_additionalDiscrimCsvColIdx = itMap->second;
            }
            return true;
        }
    }

    m_is2019FileFormat = 0;
    return false;
}


bool LtuCountryInfo::CheckColumnsInHeader(const MapHdrIdx& header, const std::vector<std::string> &cols) {
    MapHdrIdx::const_iterator itMap;
    for(size_t i = 0; i<cols.size();i++) {
        itMap = header.find(cols[i]);
        // one of the keys is not found, then exit false
        if (itMap == header.end()) {
            return false;
        }
    }
    return true;
}

bool LtuCountryInfo::Handle2019CatchCropFileLine(const MapHdrIdx& header, const std::vector<std::string>& line,
                                                 std::map<std::string, CCPracticeDatesInfos> &targetMap)
{
    std::string uid;
    for(size_t i = 0; i<m_2019CCFileCsvKeys.size();i++) {
        MapHdrIdx::const_iterator itMap = header.find(m_2019CCFileCsvKeys[i]);
        if (itMap != header.end() && itMap->second < line.size()) {
            if (i != 0) {
                uid += "-";
            }
            uid += trim(line[itMap->second], " ");
        }
    }

    CCPracticeDatesInfos ccPracticeDatesInfos;
    MapHdrIdx::const_iterator itMap;

    itMap = header.find("P_START");
    if (itMap != header.end() && itMap->second < line.size()) {
        const std::string date = NormalizeDateFormat(trim(line[itMap->second], " "));
        time_t ttTime = GetTimeFromString(date, "%Y-%m-%d");
        ccPracticeDatesInfos.pStart = TimeToString(ttTime);
    } else {
        std::cout << "Error reading CC practice dates start date!" << std::endl;
        return false;
    }

    itMap = header.find("P_END");
    if (itMap != header.end() && itMap->second < line.size()) {
        const std::string date = NormalizeDateFormat(trim(line[itMap->second], " "));
        time_t ttTime = GetTimeFromString(date, "%Y-%m-%d");
        ccPracticeDatesInfos.pEnd = TimeToString(ttTime);
    } else {
        std::cout << "Error reading CC practice dates end date!" << std::endl;
        return false;
    }

    targetMap[uid] = ccPracticeDatesInfos;

    return true;
}

std::string LtuCountryInfo::NormalizeDateFormat(const std::string &date) {
    const std::vector<std::string> &items = split(date, '/');
    if (items.size() == 3) {
        int month = std::atoi(items[0].c_str());
        int day = std::atoi(items[1].c_str());
        int year = std::atoi(items[2].c_str());
        char buff[100];
        snprintf(buff, sizeof(buff), "%04d-%02d-%02d", year, month, day);
        return std::string(buff);
    }
    return date;
}

