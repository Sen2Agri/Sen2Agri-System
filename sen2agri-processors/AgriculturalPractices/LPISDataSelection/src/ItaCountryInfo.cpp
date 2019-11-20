#include "ItaCountryInfo.h"
#include "../../Common/include/CommonFunctions.h"

ItaCountryInfo::ItaCountryInfo() {
    using namespace std::placeholders;
    m_LineHandlerFnc = std::bind(&ItaCountryInfo::HandleEfaCropsDescrFile, this, _1, _2, _3);
}

std::string ItaCountryInfo::GetName() { return "ITA"; }

std::string ItaCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    if (m_year == "2018") {
        if (m_practice =="NA") {
            // If the practice is NA, then we should not write these items
            int efaCrop = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("c_efa_crop")));
            if (efaCrop != 0) {
                return "";
            }
        }
    } else {
        if (m_practice =="NA") {
            // If the practice is NA, then we should not write these items
            if (CheckCtNumValue(&m_nfcCtNums, ogrFeat) || CheckCtNumValue(&m_flCtNums, ogrFeat)) {
                return "";
            }
        }

    }
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("ori_crop"));
}
bool ItaCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (m_year == "2018") {
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
    } else {
        std::map<int, int> *pMap = NULL;
        if (practice == NITROGEN_FIXING_CROP_VAL) {
            pMap = &m_nfcCtNums;
        } else if (practice == FALLOW_LAND_VAL) {
            pMap = &m_flCtNums;
        }
        if (pMap && CheckCtNumValue(pMap, ogrFeat)) {
            return true;
        }
    }
    return false;
}

int ItaCountryInfo::HandleEfaCropsDescrFile(const MapHdrIdx &header, const std::vector<std::string> &line, int ) {
    MapHdrIdx::const_iterator itEfaCrop = header.find("Efa_crop");
    if (itEfaCrop != header.end() && itEfaCrop->second < line.size()) {
        const std::string &efaCrop = trim(line[itEfaCrop->second], " \"");
        std::map<int, int> *pMap = NULL;
        if (efaCrop == "NFX") {
            pMap = &m_nfcCtNums;
        } else if (efaCrop == "Fallow") {
            pMap = &m_flCtNums;
        }
        if (pMap) {
            int ctnumVal = GetCtNumValue(header, line);
            if (ctnumVal > 0) {
                (*pMap)[ctnumVal] = ctnumVal;
            }
        }
    }

    return true;
}

int ItaCountryInfo::GetCtNumValue(const MapHdrIdx &header, const std::vector<std::string> &line) {
    MapHdrIdx::const_iterator itCtNum = header.find("CTnum");
    if (itCtNum != header.end() && itCtNum->second < line.size()) {
        const std::string &ctnumVal = trim(line[itCtNum->second], " \"");
        return std::atoi(ctnumVal.c_str());
    }
    return -1;
}

bool ItaCountryInfo::CheckCtNumValue(std::map<int, int> *pMap, const AttributeEntry &ogrFeat) {
    const std::string &ctnum = GetCTnum(ogrFeat);
    if (pMap->find(std::atoi(ctnum.c_str())) != pMap->end()) {
        return true;
    }
    return false;
}
