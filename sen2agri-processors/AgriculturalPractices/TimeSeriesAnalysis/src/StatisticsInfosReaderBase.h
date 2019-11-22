#ifndef StatisticsInfosReaderBase_h
#define StatisticsInfosReaderBase_h

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "TimeSeriesAnalysisTypes.h"

// #define INPUT_FILE_DATE_PATTERN     "%4d-%2d-%2d"

//typedef struct InputFileLineInfo {
//    std::string fielId;

//    time_t ttDate;              // needed for sorting the initial values
//    std::string strDate;        // TODO: see if really needed, ttDate is used instead to avoid working on strings
//    time_t ttDateFloor;
//    int weekNo;                 // TODO: to see if is really needed

//    double stdDev;
//    double meanVal;
//    double meanValChange;       // The mean value difference from the previous line

//    time_t ttDate2;             // optional - TODO: to see if is really needed
//    std::string strDate2;       // optional (TODO: to see if is really needed, ttDate2 is used instead to avoid working on strings)
//    time_t ttDate2Floor;        // optional
//    int weekNo2;                // optional - TODO: to see if is really needed

//    double GetValue() const {
//        return meanVal;
//    }

//    bool GetChangeValue(double &retVal) const {
//        retVal = meanValChange;
//        return true;
//    }

//    time_t GetFloorTime() const {
//        return ttDateFloor;
//    }

//} InputFileLineInfoType;

//struct InputFileLineInfoComparator
//{
//    inline bool operator() (const InputFileLineInfoType& struct1, const InputFileLineInfoType& struct2)
//    {
//        return (struct1.ttDate < struct2.ttDate);
//    }
//};


class StatisticsInfosReaderBase
{
public:
    StatisticsInfosReaderBase() : m_minReqEntries(0), m_bUseDate2(true), m_bSwitchDates(true), m_year(0), m_bDebug(false)
    {
    }

    virtual ~StatisticsInfosReaderBase()
    {
    }

    virtual void Initialize(const std::string &source, const std::vector<std::string> &filters, int year) = 0;

    virtual std::string GetName() = 0;

    inline void SetMinRequiredEntries(int min) { m_minReqEntries = min; }
    inline void SetUseDate2(int bUse) { m_bUseDate2 = bUse; }
    inline void SetSwitchDates(int bSwitch) { m_bSwitchDates = bSwitch; }
    inline void SetDebugMode(bool bDebug)  { m_bDebug = bDebug; }
    inline virtual bool GetEntriesForField(const std::string &fieldId, std::vector<InputFileLineInfoType> &retVect)
    {
        std::map<std::string, std::vector<InputFileLineInfoType>> retMap;
        std::map<std::string, std::vector<InputFileLineInfoType>>::const_iterator it;
        std::vector<std::string> filters;
        if (!GetEntriesForField(fieldId, filters, retMap)) {
            return false;
        }
        std::string emptyF;
        it = retMap.find(emptyF);
        if (it != retMap.end()) {
            retVect.insert(retVect.end(), it->second.begin(), it->second.end());
            return true;
        }
        return false;
    }


    virtual bool GetEntriesForField(const std::string &fieldId, const std::vector<std::string> &filters,
                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap) = 0;


protected:
    std::string m_source;
    int m_minReqEntries;
    bool m_bUseDate2;
    bool m_bSwitchDates;
    int m_year;
    bool m_bDebug;

    struct InputFileLineInfoComparator
    {
        inline bool operator() (const InputFileLineInfoType& struct1, const InputFileLineInfoType& struct2)
        {
            return (struct1.ttDate < struct2.ttDate);
        }
    };
};

#endif
