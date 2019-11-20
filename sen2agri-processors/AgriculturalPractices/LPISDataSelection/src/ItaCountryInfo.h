#ifndef ItaCountryInfo_H
#define ItaCountryInfo_H

#include "CountryInfoBase.h"

class ItaCountryInfo : public CountryInfoBase {
public:
    ItaCountryInfo();
    virtual std::string GetName();
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

private:
    int HandleEfaCropsDescrFile(const MapHdrIdx &header, const std::vector<std::string> &line, int );
    int GetCtNumValue(const MapHdrIdx &header, const std::vector<std::string> &line);
    bool CheckCtNumValue(std::map<int, int> *pMap, const AttributeEntry &ogrFeat);
    std::map<int, int> m_flCtNums;
    std::map<int, int> m_nfcCtNums;
};

#endif
