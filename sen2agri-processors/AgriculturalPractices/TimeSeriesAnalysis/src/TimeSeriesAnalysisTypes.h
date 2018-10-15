#ifndef TimeSeriesAnalysisTypes_h
#define TimeSeriesAnalysisTypes_h

#include <string>
#include <vector>
#include <iostream>

typedef struct {
    std::string fieldId;

    time_t ttDate;              // needed for sorting the initial values
    std::string strDate;        // TODO: see if really needed, ttDate is used instead to avoid working on strings
    time_t ttDateFloor;
    int weekNo;                 // TODO: to see if is really needed

    double stdDev;
    double meanVal;
    double meanValChange;       // The mean value difference from the previous line

    time_t ttDate2;             // optional - TODO: to see if is really needed
    std::string strDate2;       // optional (TODO: to see if is really needed, ttDate2 is used instead to avoid working on strings)
    time_t ttDate2Floor;        // optional
    int weekNo2;                // optional - TODO: to see if is really needed

    void Reset() {
        fieldId.clear();
        ttDate = 0;
        strDate.clear();
        ttDateFloor = 0;
        weekNo = 0;
        stdDev = 0;
        meanVal = 0;
        meanValChange = 0;
        ttDate2 = 0;
        strDate2.clear();
        ttDate2Floor = 0;
        weekNo2 = 0;
    }

    double GetValue() const {
        return meanVal;
    }

    bool GetChangeValue(double &retVal) const {
        retVal = meanValChange;
        return true;
    }

    time_t GetFloorTime() const {
        return ttDateFloor;
    }

} InputFileLineInfoType;

typedef struct MergedDateAmplitude {
    InputFileLineInfoType vvInfo;
    InputFileLineInfoType vhInfo;
    double ampRatio;
    time_t ttDate;


    double GetValue() const {
        return ampRatio;
    }

    bool GetChangeValue(double &retVal) const {
        retVal = 0.0;
        return false;
    }

    time_t GetFloorTime() const {
        return vvInfo.ttDateFloor;
    }
} MergedDateAmplitudeType;

typedef struct {
    time_t ttDate;                  // current timestamp
    double meanVal;                 // mean value
    double ampChange;               // difference from with the previous 3 weeks mean ratio - amplitude only
    double maxVal;                  // maximum value in a 7 days interval                   - coherence only
    double maxChangeVal;            // maximum difference from the previous value           - coherence only
    double sum;                     // intermediate for computing mean val - can be done outside
    size_t cnt;                     // intermediate for computing mean val - can be done outside
} GroupedMeanValInfosType;

struct GroupedMeanValInfosComparator
{
    inline bool operator() (const GroupedMeanValInfosType& struct1, const GroupedMeanValInfosType& struct2)
    {
        return (struct1.ttDate < struct2.ttDate);
    }
};

typedef struct {
    time_t ttDate;

    double cohMax;                  // coherence max value in week week
    double cohChange;               // coherence change from the previous value

    double ampMean;                 // amplitude week mean
    double ampMax;                  // amplitude max value in week
    double ampChange;               // ammplitude ration difference from with the previous 3 weeks mean

    double ndviMeanVal;
    double ndviPrev;
    double ndviNext;
    short vegWeeks;                // TODO: I think this could be converted to boolean with only TRUE/FALSE values TBC
    short ndviPresence;
    double ndviDrop;

    bool candidateOptical;

    bool coherenceBase;
    bool coherenceHigh;
    bool coherencePresence;
    bool candidateCoherence;

    bool candidateAmplitude;
    bool amplitudePresence;

    bool harvest;

    bool candidateEfa;
    bool catchStart;

} MergedAllValInfosType;

typedef struct {
    std::string fieldId;
    int year;
    int mainCrop;
    time_t ttVegStartTime;
    time_t ttHarvestStartTime;
    time_t ttHarvestEndTime;
    std::string practiceName;
    time_t ttPracticeStartTime;
    time_t ttPracticeEndTime;

    short ndviPresence;             // M1 / M6
    bool candidateOptical;          // M2
    bool candidateAmplitude;        // M3
    bool amplitudePresence;         // M4
    bool candidateCoherence;        // M5
    short harvestConfirmWeek;       // WEEK

    std::string efaIndex;           // C_INDEX
    short ndviGrowth;               // M7
    double ndviNoLoss;              // M8
    double ampNoLoss;               // M9
    double cohNoLoss;               // M10

} HarvestEvaluationType;

typedef struct {
    time_t harvestConfirmed;
    HarvestEvaluationType evaluation;

} HarvestInfoType;

typedef struct {
    time_t ttDate;
    short ndviPresence;
    short ndviDrop;
    double ndviMean;
    short ndviGrowth;
    short ndviNoLoss;

    short ampNoLoss;

    short cohNoLoss;

} EfaMarkersInfoType;

typedef struct FieldInfoType {

    FieldInfoType(const std::string &fid) {
        fieldId = fid;
    }
    std::string fieldId;
    std::string countryCode;
    std::string mainCrop;
    std::string practiceType;

    int year;                                   // TODO: Check that it is extracted from the YEAR field of the shape
    time_t ttVegStartTime;
    time_t ttVegStartWeekFloorTime;
    time_t ttHarvestStartTime;
    time_t ttHarvestStartWeekFloorTime;
    time_t ttHarvestEndTime;
    time_t ttHarvestEndWeekFloorTime;

    std::string practiceName;
    time_t ttPracticeStartTime;
    time_t ttPracticeStartWeekFloorTime;
    time_t ttPracticeEndTime;
    time_t ttPracticeEndWeekFloorTime;

    int vegStartWeekNo;
    int harvestStartWeekNo;

    std::vector<InputFileLineInfoType> ampVVLines;
    std::vector<InputFileLineInfoType> ampVHLines;
    std::vector<MergedDateAmplitudeType> mergedAmpInfos;

    std::vector<InputFileLineInfoType> coheVVLines;
    std::vector<InputFileLineInfoType> coheVHLines;     // TODO: These are read but not actually used anywhere. To be removed (???)
    double coheVVMaxValue;                              // Needed for marker 5

    std::vector<InputFileLineInfoType> ndviLines;

    std::vector<GroupedMeanValInfosType> ampRatioGroups;
    std::vector<GroupedMeanValInfosType> ndviGroups;
    std::vector<GroupedMeanValInfosType> coherenceGroups;

} FieldInfoType;


#endif
