#ifndef NlCountryInfo_H
#define NlCountryInfo_H

#include "CountryInfoBase.h"

class NlCountryInfo  : public CountryInfoBase {
public:
    virtual void InitializeIndexes(OGRFeature &firstOgrFeat);
    virtual std::string GetName();
    virtual std::string GetUniqueId(OGRFeature &ogrFeat);
    virtual std::string GetMainCrop(OGRFeature &ogrFeat);
    virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice);

    virtual std::string GetHStart(OGRFeature &ogrFeat);

    virtual std::string GetPractice(OGRFeature &ogrFeat);

    virtual std::string GetPracticeType(OGRFeature &ogrFeat);
    virtual std::string GetPStart(OGRFeature &ogrFeat);
    virtual std::string GetPEnd(OGRFeature &ogrFeat);

private:
    int m_FUNCTIONEE_FieldIdx;
    int m_GRONDBEDEK_FieldIdx;
    int m_IND_EA_FieldIdx;
    int m_GRONDBED_2_FieldIdx;
};

#endif
