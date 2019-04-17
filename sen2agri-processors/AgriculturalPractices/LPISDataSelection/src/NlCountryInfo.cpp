#include "NlCountryInfo.h"

std::string NlCountryInfo::GetName() { return "NL"; }

void NlCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);

    m_FUNCTIONEE_FieldIdx = firstOgrFeat.GetFieldIndex("FUNCTIONEE");
    m_GRONDBEDEK_FieldIdx = firstOgrFeat.GetFieldIndex("GRONDBEDEK");
    m_IND_EA_FieldIdx = firstOgrFeat.GetFieldIndex("IND_EA");
    m_GRONDBED_2_FieldIdx = firstOgrFeat.GetFieldIndex("GRONDBED_2");
}

std::string NlCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(m_FUNCTIONEE_FieldIdx);
}

std::string NlCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    return ogrFeat.GetFieldAsString(m_GRONDBEDEK_FieldIdx);
}

bool NlCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) {
    if (practice == CATCH_CROP_VAL) {
        std::string field = ogrFeat.GetFieldAsString(m_IND_EA_FieldIdx);
        if (field == "J") {
            return true;
        }
        field = ogrFeat.GetFieldAsString(m_GRONDBED_2_FieldIdx);
        if (field.size() > 0) {
            return true;
        }
    }

    return false;
}

std::string NlCountryInfo::GetHStart(const AttributeEntry &ogrFeat) {
    const std::string &mainCrop = GetMainCrop(ogrFeat);
    if (mainCrop == "233") {
        return m_hWinterStart;
    }
    return m_hstart;
}

std::string NlCountryInfo::GetPractice(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        std::string field = ogrFeat.GetFieldAsString(m_IND_EA_FieldIdx);
        if (field == "J") {
            return "CatchCropIsMain";
        }
        field = ogrFeat.GetFieldAsString(m_GRONDBED_2_FieldIdx);
        if (field.size() > 0) {
            return m_practice;
        }
        return "NA";
    }
    // NO NFC or Fallow for NLD

    return m_practice;
}

std::string NlCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        std::string field = ogrFeat.GetFieldAsString(m_GRONDBED_2_FieldIdx);
        std::string retPtype = field;
        if (field.size() > 0) {
            retPtype = m_practice + "_" + std::to_string(std::atoi(field.c_str()));
        }
        const std::string &practice = GetPractice(ogrFeat);
        if (practice == "CatchCropIsMain") {
            retPtype = "CatchCropIsMain" + retPtype;
        }
        return retPtype;
    }
    return "NA";
}
std::string NlCountryInfo::GetPStart(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        const std::string &practice = GetPractice(ogrFeat);
        if (practice == "CatchCropIsMain") {
            return m_pstart;
        } else if (practice != "NA") {
            return m_pWinterStart;
        }
    }
    return "NA";
}
std::string NlCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL) {
        const std::string &practice = GetPractice(ogrFeat);
        if (practice == "CatchCropIsMain") {
            return m_pend;
        } else {
            return "NA";
        }
    }
    return "NA";

}
