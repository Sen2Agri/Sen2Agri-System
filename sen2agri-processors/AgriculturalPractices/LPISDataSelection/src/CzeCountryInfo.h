#ifndef CzeCountryInfo_H
#define CzeCountryInfo_H

#include "CountryInfoBase.h"

class CzeCountryInfo : public CountryInfoBase {
public:
    CzeCountryInfo();

    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetName();
    virtual std::string GetOriId(const AttributeEntry &ogrFeat);
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);

    virtual std::string GetHEnd(const AttributeEntry &ogrFeat);

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
    int Handle2018FileLine(const MapHdrIdx &header, const std::vector<std::string> &line, int fileIdx);
    int Handle2019FileLine(const MapHdrIdx &header, const std::vector<std::string> &line, int);

    LpisInfosType GetLpisInfos(const std::string &fid);
    EfaInfosType GetEfaInfos(const std::string &fid);

    int m_NKOD_DPB_FieldIdx;
};

#endif
