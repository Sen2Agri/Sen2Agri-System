#ifndef StatisticsInfosSingleCsvReader_h
#define StatisticsInfosSingleCsvReader_h

#include "StatisticsInfosReaderBase.h"
#include <inttypes.h>

class StatisticsInfosSingleCsvReader : public StatisticsInfosReaderBase
{

    typedef struct {
        std::string suffix;
        uintmax_t startIdx;
        unsigned int len;
    } FieldIndexInfos;

    typedef std::map<std::string, std::vector<FieldIndexInfos>> IdxMapType;

public:
    StatisticsInfosSingleCsvReader();

   virtual  ~StatisticsInfosSingleCsvReader()
    {
    }
    virtual void Initialize(const std::string &source, const std::vector<std::string> &filters, int year);
    virtual std::string GetName() { return "csv"; }

    virtual bool GetEntriesForField(const std::string &fid, const std::vector<std::string> &filters,
                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);


private:

    std::vector<std::string> GetInputFileLineElements(const std::string &line);
    bool ExtractLinesFromStream(std::istream &inStream, const std::string &fieldId,
                                const std::vector<std::string> &findFilters,
                                std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);
    bool ExtractInfosFromLine(const std::string &fileLine, const std::vector<std::string> &findFilters,
                              std::vector<InputFileLineInfoType> &lineInfos, std::string &uid);

private:
    size_t m_InputFileHeaderLen;
    size_t m_CoheInputFileHeaderLen;

    std::string m_strSource;
    IdxMapType m_IdxMap;
    bool m_bIsCoheFile;
};

#endif
