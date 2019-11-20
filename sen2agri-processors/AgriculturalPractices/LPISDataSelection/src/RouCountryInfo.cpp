#include "RouCountryInfo.h"
#include "../../Common/include/CommonFunctions.h"

#define PRACTICE_END_OFFSET 4752000     // 55 days


RouCountryInfo::RouCountryInfo() {
    using namespace std::placeholders;
    m_ShpFeatHandlerFnc = std::bind(&RouCountryInfo::HandleCCFeature, this, _1, _2);
    m_LineHandlerFnc = std::bind(&RouCountryInfo::HandleCCParcelsDescrFile, this, _1, _2, _3);
}

std::string RouCountryInfo::GetName() { return "ROU"; }

std::string RouCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return GetOriCrop(ogrFeat);
}

bool RouCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (practice == CATCH_CROP_VAL) {
        const std::string &uid = GetOriId(ogrFeat);
        if (m_ccGsaaIdsMap.find(uid) != m_ccGsaaIdsMap.end()) {
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
    if (m_practice == CATCH_CROP_VAL) {
        if (m_year == "2019") {
            const std::string &uid = GetOriId(ogrFeat);
            std::map<std::string, GsaaInfoType> ::iterator gsaaMapIt = m_ccGsaaIdsMap.find(uid);
            if (gsaaMapIt != m_ccGsaaIdsMap.end()) {
                return gsaaMapIt->second.pStartDate;
            } else {
                return "NA";
            }
        }

    } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        switch(cropCode) {
            case 9741:
            case 95561:
            case 97471:
            case 97481:
                return m_pWinterStart;
        }
    }
    return m_pstart;
}

std::string RouCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        if (m_year == "2019") {
            const std::string &uid = GetOriId(ogrFeat);
            std::map<std::string, GsaaInfoType> ::iterator gsaaMapIt = m_ccGsaaIdsMap.find(uid);
            if (gsaaMapIt != m_ccGsaaIdsMap.end()) {
                return gsaaMapIt->second.pEndDate;
            } else {
                return "NA";
            }
        }

    } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
        int cropCode = std::atoi(GetMainCrop(ogrFeat).c_str());
        switch(cropCode) {
            case 1511:
            case 15171:
            case 1591:
            {
                if (m_year == "2018") {
                    return "2018-06-03";
                } else if (m_year == "2019") {
                    return "2019-06-02";
                }
            }
            case 1281:
            case 1291:
            case 1301:
            {
                if (m_year == "2018") {
                    return "2018-08-26";
                } else if (m_year == "2019") {
                    return "2019-09-01";
                }
            }
            case 9731:
            case 95531:
            case 95541:
            case 9751:
                return (m_year + "-06-30");
            case 9741:
            case 95561:
            case 97471:
            case 97481:
            {
                if (m_year == "2018") {
                    return "2018-05-06";
                } else if (m_year == "2019") {
                    return "2019-05-05";
                }
            }
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
    std::cout << gsaaId << std::endl;
    m_ccGsaaIdsMap[gsaaId] = {"NA", "NA"};
    return true;
}

int RouCountryInfo::HandleCCParcelsDescrFile(const MapHdrIdx &header,
                                             const std::vector<std::string> &line,
                                             int ) {

    MapHdrIdx::const_iterator itDataRasarireMap = header.find("data_rasarire");
    if (itDataRasarireMap != header.end() && itDataRasarireMap->second < line.size()) {
        const std::string &dataRasarire = trim(line[itDataRasarireMap->second], " \"");
        time_t ttPStart = GetTimeFromString(dataRasarire);
        if (ttPStart > 0) {
            time_t ttPEnd = (ttPStart + PRACTICE_END_OFFSET);
            const std::string &strPEnd = TimeToString(ttPEnd);
            const std::string &uniqueId = GetUidFromCCParcelDescrFile(header, line);
            std::map<std::string, GsaaInfoType> ::iterator gsaaMapIt = m_ccGsaaIdsMap.find(uniqueId);
            if(gsaaMapIt != m_ccGsaaIdsMap.end()) {
                gsaaMapIt->second.pStartDate = dataRasarire;
                gsaaMapIt->second.pEndDate = strPEnd;
            }
        }
    }
    return true;
}

std::string RouCountryInfo::GetUidFromCCParcelDescrFile(const MapHdrIdx &header,
                                             const std::vector<std::string> &line) {
    MapHdrIdx::const_iterator itIdMap = header.find("id_unic");
    MapHdrIdx::const_iterator itFbIdMap = header.find("fbid");
    MapHdrIdx::const_iterator itNrParcelaMap = header.find("nr_parcela");

    if (itIdMap != header.end() && itIdMap->second < line.size() &&
        itFbIdMap != header.end() && itFbIdMap->second < line.size() &&
        itNrParcelaMap != header.end() && itNrParcelaMap->second < line.size()) {

        const std::string &nrParcela = line[itNrParcelaMap->second];
        return line[itIdMap->second] + "-" +
                       line[itFbIdMap->second] + "-" +
                        nrParcela.substr(0, nrParcela.size() - 1) + "-" +
                        nrParcela.substr(nrParcela.size() - 1);
    }
    return "";
}


template <typename T>
std::string RouCountryInfo::GetGSAAUniqueId(T &ogrFeat) {
    if (m_year == "2018") {
        std::string idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("ID_unic")));
        if (idUnic.size() == 0 || idUnic == "0") {
            idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("id_unic")));
        }
        const std::string &parcelNo = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("parcel_nr")));
        std::string gsaaId = idUnic + "-" + parcelNo + "-" +
                ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_nr"));

        return gsaaId;
    } else {
        const std::string &idUnic = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("old_id")));
        const std::string &sirsupCod = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("sirsup_cod")));
        const std::string &blocNo = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("bloc_nr")));
        const std::string &parcelNo = std::to_string(ogrFeat.GetFieldAsInteger(ogrFeat.GetFieldIndex("parcel_nr")));
        const std::string &cropNo = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("crop_nr"));
        return (idUnic + "-" + sirsupCod + "-" + blocNo + "-" + parcelNo + "-" + cropNo);
    }
}

