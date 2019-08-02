#ifndef TsaPrevPrdReader_h
#define TsaPrevPrdReader_h

#include "TimeSeriesAnalysisTypes.h"
#include <map>

class TsaPrevPrdReader
{
public:
    TsaPrevPrdReader();
    bool Initialize(const std::string &prevPrd);
    std::string GetHWeekForFieldId(int fieldId);
    std::string GetHWeekStartForFieldId(int fieldId);

private:
    void ExtractHeaderInfos(const std::string &line);
    std::ifstream m_FileStream;
    std::vector<std::string> m_InputFileHeader;
    int m_fieldIdPos;
    int m_hWeekPos;
    int m_hWeekStartPos;

    typedef struct FieldInfos {
        std::string hWeek;
        std::string hWeekStart;
    } FieldInfos;
    std::map<int, FieldInfos> m_MapIdHendWeek;
};

#endif
