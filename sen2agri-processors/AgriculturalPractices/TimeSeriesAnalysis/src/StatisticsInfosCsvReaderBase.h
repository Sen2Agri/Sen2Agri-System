#ifndef StatisticsInfosFolderFilesReader_h
#define StatisticsInfosFolderFilesReader_h

#include "StatisticsInfosReaderBase.h"

typedef struct {
    std::string fileName;
    std::string filePath;
} FileInfoType;


class StatisticsInfosCsvReaderBase : public StatisticsInfosReaderBase
{

public:
    StatisticsInfosCsvReaderBase();

   virtual  ~StatisticsInfosCsvReaderBase()
    {
    }
    virtual void Initialize(const std::string &source, const std::vector<std::string> &filters);
    virtual std::string GetName() { return "dir"; }

    virtual bool GetEntriesForField(const std::string &fieldId, const std::vector<std::string> &filters,
                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);


private:

    std::vector<FileInfoType> GetFilesInFolder(const std::string &targetPath);
    std::vector<FileInfoType> FindFilesForFieldId(const std::string &fieldId);
    std::vector<std::string> GetInputFileLineElements(const std::string &line);
    bool ExtractFileInfosForFilter(const FileInfoType &fileInfo, const std::string &filter,
                                   std::map<std::string, std::vector<InputFileLineInfoType>> &retMap);
    bool ExtractInfosFromLine(const std::string &fileLine, InputFileLineInfoType &lineInfo);
    bool ExtractLineInfos(const std::string &filePath, std::vector<InputFileLineInfoType> &retValidLines);

private:
    std::vector<FileInfoType> m_InfoFiles;
    std::vector<std::string> m_InputFileHeader;
    std::vector<std::string> m_CoheInputFileHeader;
};

#endif
