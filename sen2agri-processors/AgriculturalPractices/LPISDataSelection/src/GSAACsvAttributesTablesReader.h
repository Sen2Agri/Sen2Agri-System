#ifndef GSAACsvAttributesTablesReader_h
#define GSAACsvAttributesTablesReader_h

#include <vector>
#include <map>
#include <fstream>

#include "GSAAAttributesTablesReaderBase.h"

class GSAACsvAttributesTablesReader : public GSAAAttributesTablesReaderBase
{
public:
    virtual std::string GetName() {return "csv";}
    virtual bool ExtractAttributes(std::function<void (const AttributeEntry&)> fnc);

private:
    class CsvFeatureDescription : public AttributeEntry
    {
        CsvFeatureDescription() {
        }
        virtual int GetFieldIndex(const char *pszName) const;
        virtual const char* GetFieldAsString(int idx) const;
        virtual double GetFieldAsDouble(int idx) const;
        virtual int GetFieldAsInteger(int idx) const;

    private:

        bool ExtractLineInfos(const std::string &line);
        std::vector<std::string> LineToVector(const std::string &line);
        bool ExtractHeaderInfos(const std::string &line);


        std::ifstream m_fStream;
        std::map<std::string, int> m_InputFileHeader;
        std::vector<std::string> m_lineEntries;

        bool m_bIsValid;
        std::string m_source;

        friend class GSAACsvAttributesTablesReader;
    };
};

#endif
