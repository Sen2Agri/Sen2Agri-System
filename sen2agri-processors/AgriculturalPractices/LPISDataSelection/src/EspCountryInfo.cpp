#include "EspCountryInfo.h"

EspCountryInfo::EspCountryInfo() : m_cDeclaraFieldIdx(-1), m_cProductoFieldIdx(-1),
    m_cVariedadFieldIdx(-1), m_ctNumFieldIdx(-1)
{
    using namespace std::placeholders;
    m_LineHandlerFnc = std::bind(&EspCountryInfo::HandleFileLine, this, _1, _2, _3);
}

std::string EspCountryInfo::GetName() { return "ESP"; }

bool EspCountryInfo::IsMonitoringParcel(const AttributeEntry &ogrFeat) {
    if (!CountryInfoBase::IsMonitoringParcel(ogrFeat)) {
        return false;
    }
    if (GetYear() == "2018") {
        return true;
    }

    int ctNumCode = std::atoi(ogrFeat.GetFieldAsString(m_ctNumFieldIdx));
    if (m_fallowCropCodes.find(ctNumCode) != m_fallowCropCodes.end() ||
        m_nfcCropCodes.find(ctNumCode) != m_nfcCropCodes.end() ||
        m_naCropCodes.find(ctNumCode) != m_naCropCodes.end()) {
        return true;
    }
    return false;
}

void EspCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);
    m_cDeclaraFieldIdx = firstOgrFeat.GetFieldIndex("ori_id");
    m_cProductoFieldIdx = firstOgrFeat.GetFieldIndex("ori_crop");
    m_cVariedadFieldIdx = firstOgrFeat.GetFieldIndex("C_VARIEDAD");
    m_ctNumFieldIdx = firstOgrFeat.GetFieldIndex("CTnum");
    if (GetYear() == "2018") {
        m_nfcCropCodes = {{34,   34}, {40,   40}, {41,   41}, {43,   43}, {49,   49}, {50,   50}, {51,   51}, {52,   52}, {53,   53}, {60,   60},
        {61,   61}, {67,   67}, {76,   76}, {87,   87}, {180,  180}, {238,  238}, {239,  239}, {240,  240}, {248,  248}, {249,  249},
        {250,  250}, {77,   77 }, {241,  241}, {242,  242}, {243,  243}, {244,  244}, {245,  245}, {246,  246}, {298,  298}, {299,  299},
        {336,  336}, {337,  337}, {338,  338}, {339,  339}, {340,  340}, {342,   342}};
    }
}

std::string EspCountryInfo::GetOriId(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(m_cDeclaraFieldIdx);
}
std::string EspCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return RemoveSuffix(ogrFeat.GetFieldAsString(m_cProductoFieldIdx));
}
bool EspCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (GetYear() == "2018") {
        if (practice == FALLOW_LAND_VAL) {
            int cropCode = std::atoi(ogrFeat.GetFieldAsString(m_cProductoFieldIdx));
            int variedad = std::atoi(ogrFeat.GetFieldAsString(m_cVariedadFieldIdx));
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
             int cropCode = std::atoi(ogrFeat.GetFieldAsString(m_cProductoFieldIdx));
             if (m_nfcCropCodes.find(cropCode) != m_nfcCropCodes.end()) {
                 return true;
             }
        }
    } else {
        int ctNumVal = std::atoi(ogrFeat.GetFieldAsString(m_ctNumFieldIdx));
        if (practice == FALLOW_LAND_VAL) {
            if (m_fallowCropCodes.find(ctNumVal) != m_fallowCropCodes.end()) {
                 return true;
            }
        } else if(practice == NITROGEN_FIXING_CROP_VAL) {
            if (m_nfcCropCodes.find(ctNumVal) != m_nfcCropCodes.end()) {
                return true;
            }
        }
    }
    return false;
}

std::string EspCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    if (GetYear() == "2018") {
        if (m_practice == FALLOW_LAND_VAL) {
            return ogrFeat.GetFieldAsString(m_cVariedadFieldIdx);
        } // else if (practice == NITROGEN_FIXING_CROP_VAL) return "NA"
    }
    return "NA";
}

int EspCountryInfo::HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int) {
    MapHdrIdx::const_iterator itMapCtNum;
    MapHdrIdx::const_iterator itFallow;
    MapHdrIdx::const_iterator itNfc;
    MapHdrIdx::const_iterator itNA;

    itMapCtNum = header.find("CTnum");
    itFallow = header.find("Fallow");
    itNfc = header.find("NFC");
    itNA = header.find("Harvest");
    if (itMapCtNum != header.end() && itMapCtNum->second < line.size() &&
        itFallow != header.end() && itFallow->second < line.size() &&
        itNfc != header.end() && itNfc->second < line.size() &&
        itNA != header.end() && itNA->second < line.size()) {
        int ctNumVal = std::atoi(line[itMapCtNum->second].c_str());
        if (boost::iequals(line[itFallow->second], "yes")) {
            m_fallowCropCodes[ctNumVal] = ctNumVal;
        } else if (boost::iequals(line[itNfc->second], "yes")) {
            m_nfcCropCodes[ctNumVal] = ctNumVal;
        } else if (boost::iequals(line[itNA->second], "yes")) {
            m_naCropCodes[ctNumVal] = ctNumVal;
        }
    } else {
        return false;
    }
    return true;
}

std::string EspCountryInfo::RemoveSuffix(const std::string &field) {
    size_t lastindex = field.find_last_of(".");
    if (lastindex != field.npos) {
        return field.substr(0, lastindex);
    }
    return field;
}

