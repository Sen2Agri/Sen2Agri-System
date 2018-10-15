#ifndef StatisticsInfosXmlReaderBase_h
#define StatisticsInfosXmlReaderBase_h

#include "StatisticsInfosReaderBase.h"
#include "expat.h"

class StatisticsInfosXmlReader : public StatisticsInfosReaderBase
{

public:
    StatisticsInfosXmlReader();

   virtual  ~StatisticsInfosXmlReader()
    {
    }
    virtual void SetSource(const std::string &source);
    virtual std::string GetName() { return "xml"; }

    virtual bool GetEntriesForField(const std::string &inFieldId, const std::vector<std::string> &filters,
                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);


private:
    std::string m_strSource;

//    std::vector<FileInfoType> GetFilesInFolder(const std::string &targetPath);
//    std::vector<FileInfoType> FindFilesForFieldId(const std::string &fieldId);
//    std::vector<std::string> GetInputFileLineElements(const std::string &line);
//    bool ExtractFileInfosForFilter(const FileInfoType &fileInfo, const std::string &filter,
//                                   std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);
//    bool ExtractInfosFromLine(const std::string &fileLine, InputFileLineInfoType &lineInfo);
//    bool ExtractLineInfos(const std::string &filePath, std::vector<InputFileLineInfoType> &retValidLines);

private:
//    std::vector<FileInfoType> m_InfoFiles;
//    std::vector<std::string> m_InputFileHeader;
//    std::vector<std::string> m_CoheInputFileHeader;

    XML_Parser m_Parser;

};

#endif
