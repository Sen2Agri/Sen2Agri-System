#ifndef CzeCountryInfo_H
#define CzeCountryInfo_H

#include "CountryInfoBase.h"

class CzeCountryInfo : public CountryInfoBase {
public:
    CzeCountryInfo();

    virtual void InitializeIndexes(OGRFeature &firstOgrFeat);
    virtual std::string GetName();
    virtual std::string GetUniqueId(OGRFeature &ogrFeat);
    virtual std::string GetMainCrop(OGRFeature &ogrFeat);
    virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(OGRFeature &ogrFeat);
    virtual std::string GetPStart(OGRFeature &ogrFeat);
    virtual std::string GetPEnd(OGRFeature &ogrFeat);

private :
    typedef struct {
        std::string plod1;
        std::string plod2;
        std::string vymera;
    } LpisInfosType;

    typedef struct {
        std::string typ_efa;
        std::string vym_efa;
        std::string var_mpl;
    } EfaInfosType;

    std::map<std::string, EfaInfosType> efaInfosMap;
    std::map<std::string, LpisInfosType> lpisInfosMap;

    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);
    LpisInfosType GetLpisInfos(const std::string &fid);
    EfaInfosType GetEfaInfos(const std::string &fid);

    int m_NKOD_DPB_FieldIdx;
};

#endif
