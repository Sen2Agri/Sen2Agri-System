#ifndef NlCountryInfo_H
#define NlCountryInfo_H

#include "CountryInfoBase.h"

class NlCountryInfo  : public CountryInfoBase {
public:
    NlCountryInfo();
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetName();
    virtual std::string GetOriId(const AttributeEntry &ogrFeat);
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetHStart(const AttributeEntry &ogrFeat);

    virtual std::string GetPractice(const AttributeEntry &ogrFeat);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);

    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int);

private:
    int m_FUNCTIONEE_FieldIdx;
    int m_GRONDBEDEK_FieldIdx;
    int m_IND_EA_FieldIdx;
    int m_GRONDBED_2_FieldIdx;

    std::map<std::string, std::string> m_mainCropToHStartMap;
};

#endif
