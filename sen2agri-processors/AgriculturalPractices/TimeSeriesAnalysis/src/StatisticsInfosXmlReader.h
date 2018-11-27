#ifndef StatisticsInfosXmlReaderBase_h
#define StatisticsInfosXmlReaderBase_h

#include "StatisticsInfosReaderBase.h"
#include "expat.h"
#include <inttypes.h>

class StatisticsInfosXmlReader : public StatisticsInfosReaderBase
{

    typedef struct {
        std::string name;
        uintmax_t startIdx;
        unsigned int len;
    } FieldIndexInfos;

    typedef std::map<std::string, std::vector<FieldIndexInfos>> IdxMapType;

public:
    StatisticsInfosXmlReader();

   virtual  ~StatisticsInfosXmlReader()
    {
    }
    virtual void Initialize(const std::string &source, const std::vector<std::string> &filters, int year);
    virtual std::string GetName() { return "xml"; }

    virtual bool GetEntriesForField(const std::string &inFieldId, const std::vector<std::string> &filters,
                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);


private:
    std::string m_strSource;

private:

    XML_Parser m_Parser;
    IdxMapType m_IdxMap;
};

#endif
