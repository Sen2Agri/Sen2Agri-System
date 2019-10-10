#ifndef LtuCountryInfo_H
#define LtuCountryInfo_H

#include "CountryInfoBase.h"

class LtuCountryInfo : public CountryInfoBase {
private :
    std::map<std::string, std::string> m_ccISMap;
    std::map<std::string, std::string> m_ccPOMap;
    std::map<std::string, std::string> m_ccSPMap;
    std::map<std::string, std::string> m_greenFallowMap;
    std::map<std::string, std::string> m_blackFallowMap;
    std::map<std::string, std::string> m_nfcMap;

public:
    LtuCountryInfo();

    virtual std::string GetName();
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual std::string GetOriId(const AttributeEntry &ogrFeat);

    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat);
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice);

    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);

    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);
    virtual std::string GetHStart(const AttributeEntry &ogrFeat);

    int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);

    bool HasUid(const std::string &fid, const std::map<std::string, std::string> &refMap);

private:
    std::string GetGSAAUniqueId(const AttributeEntry &ogrFeat);
    bool Is2019FileFormat(const MapHdrIdx& header);
    bool CheckColumnsInHeader(const MapHdrIdx& header, const std::vector<std::string> &cols);
    int Handle2018FileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);
    int Handle2019FileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx);
    bool AddUuidToMap(const MapHdrIdx& header, const std::vector<std::string> &keys, const std::vector<std::string> &line,
                      std::map<std::string, std::string> *pRefMap);

    int m_PSL_KODAS_FieldIdx;

    int m_VALDOS_NR_FieldIdx;
    int m_KZS_NR_FieldIdx;
    int m_LAUKO_NR_FieldIdx;

    std::vector<std::string> m_2018FileCsvKeys;
    std::vector<std::string> m_2019FileCsvKeys;
    std::vector<std::string> m_2019CCFileCsvKeys;
    std::string m_DiscrimCsvColName;
    std::string m_AdditionalDiscrimCsvColName;
    std::string m_DiscrimPOCsvVal;
    std::string m_DiscrimSPCCCsvVal;
    std::string m_DiscrimISCsvVal;

    std::string m_DiscrimFLCsvVal;
    std::string m_DiscrimAddPDZFLCsvVal;
    std::string m_DiscrimAddPDJFLCsvVal;
    std::string m_DiscrimNFCCsvVal;

    int m_is2019FileFormat;
    int m_discrimCsvColIdx;
    int m_additionalDiscrimCsvColIdx;

    typedef struct CCPracticeDatesInfos {
        std::string pStart;
        std::string pEnd;
    } CCPracticeDatesInfos;

    std::map<std::string, CCPracticeDatesInfos> m_ccISPracticeDatesFilterMap;
    std::map<std::string, CCPracticeDatesInfos> m_ccSPPracticeDatesFilterMap;

    bool Handle2019CatchCropFileLine(const MapHdrIdx& header, const std::vector<std::string>& line,
                                     std::map<std::string, CCPracticeDatesInfos> &targetMap);
    std::string NormalizeDateFormat(const std::string &date);

};

#endif
