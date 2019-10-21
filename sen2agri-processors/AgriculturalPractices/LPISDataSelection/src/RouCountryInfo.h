#ifndef RouCountryInfo_H
#define RouCountryInfo_H

#include "CountryInfoBase.h"

class RouCountryInfo : public CountryInfoBase {
private:
    typedef struct {
        std::string pStartDate;
        std::string pEndDate;
    } GsaaInfoType;
    std::map<std::string, GsaaInfoType> m_ccGsaaIdsMap;
    const std::map<int, int> m_nfcCropCodes = {{1511 , 1511}, {15171, 15171}, {1591 , 1591}, {1521 , 1521}, {15271, 15271},
                                               {2031 , 2031}, {20371, 20371}, {1271 , 1271}, {1281 , 1281}, {1291 , 1291},
                                               {1301 , 1301}, {1531 , 1531}, {1551 , 1551}, {95591, 95591}, {1571 , 1571},
                                               {1541 , 1541}, {1561 , 1561}, {9731 , 9731}, {95531, 95531}, {95541, 95541},
                                               {9741 , 9741}, {95561, 95561}, {97471, 97471}, {97481, 97481}, {97491, 97491},
                                               {9751 , 9751}};

public:
    RouCountryInfo();

    virtual std::string GetName();
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);

    int HandleCCFeature(OGRFeature &ogrFeat, int fileIdx);
    int HandleCCParcelsDescrFile(const MapHdrIdx &header, const std::vector<std::string> &line, int);

private:
    template <typename T>
    std::string GetGSAAUniqueId(T &ogrFeat);
    std::string GetUidFromCCParcelDescrFile(const MapHdrIdx &header, const std::vector<std::string> &line);
};

#endif
