#ifndef TimeSeriesAnalysisUtils_h
#define TimeSeriesAnalysisUtils_h

#include <time.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define NOT_AVAILABLE               -10000
#define NR                          -10001
#define SEC_IN_DAY                   86400          // seconds in day = 24 * 3600
#define SEC_IN_WEEK                  604800         // seconds in week = 7 * 24 * 3600

boost::gregorian::greg_weekday const FirstDayOfWeek = boost::gregorian::Sunday;

template <typename T>
inline bool IsNA(T val) {
    return (val == NOT_AVAILABLE);
}

template <typename T>
bool IsNR(T val) {
    return (val == NR);
}

inline time_t to_time_t(const boost::gregorian::date& date ){
    if (date.is_not_a_date()) {
        return 0;
    }

    using namespace boost::posix_time;
    static ptime epoch(boost::gregorian::date(1970, 1, 1));
    time_duration::sec_type secs = (ptime(date,seconds(0)) - epoch).total_seconds();
    return time_t(secs);
}

inline boost::gregorian::date GetBoostDateFromString(const std::string &strDate) {
    return boost::gregorian::from_simple_string(strDate);
}

inline time_t GetTimeFromString(const std::string &strDate) {
    try {
        boost::gregorian::date const d = boost::gregorian::from_simple_string(strDate);
        return to_time_t(d);
    } catch (...) {
        return 0;
    }
}

inline bool ComputeSlope(const std::vector<double>& x, const std::vector<double>& y, double &slope){
    if(x.size() != y.size()) {
        return false;
    }
    double n = x.size();

    double avgX = std::accumulate(x.begin(), x.end(), 0.0) / n;
    double avgY = std::accumulate(y.begin(), y.end(), 0.0) / n;

    double numerator = 0.0;
    double denominator = 0.0;

    for(int i=0; i<n; ++i){
        numerator += (x[i] - avgX) * (y[i] - avgY);
        denominator += (x[i] - avgX) * (x[i] - avgX);
    }

    if(denominator == 0){
        return false;
    }

    slope = numerator / denominator;

    return true;
}

template <typename T>
inline std::vector<T> GetLinearFit(const std::vector<T>& xData, const std::vector<T>& yData)
{
    T xSum = 0, ySum = 0, xxSum = 0, xySum = 0, slope, intercept;
    for (long i = 0; i < yData.size(); i++)
    {
        xSum += xData[i];
        ySum += yData[i];
        xxSum += xData[i] * xData[i];
        xySum += xData[i] * yData[i];
    }
    slope = (yData.size() * xySum - xSum * ySum) / (yData.size() * xxSum - xSum * xSum);
    intercept = (ySum - slope * xSum) / yData.size();
    std::vector<T> res;
    res.push_back(slope);
    res.push_back(intercept);
    return res;
}

inline time_t FloorDateToWeekStart(time_t ttTime) {
    boost::gregorian::date bDate = boost::posix_time::from_time_t(ttTime).date();
    short dayOfTheWeek = bDate.day_of_week();
    int offsetSec;
    if (FirstDayOfWeek == boost::gregorian::Sunday) {
        // S M T W T F S
        // 0 1 2 3 4 5 6
        offsetSec = dayOfTheWeek * 24 * 3600;
    } else {
        // S M T W T F S
        // 6 0 1 2 3 4 5
        int offsetDays = (dayOfTheWeek == 0) ? 6 : dayOfTheWeek - 1;
        offsetSec = offsetDays * 24 * 3600;
    }
    return (ttTime - offsetSec);
}

/*
time_t FloorWeekDate(int year, int week)
{
    boost::gregorian::date const jan1st(year, boost::gregorian::Jan, 1);
    boost::gregorian::first_day_of_the_week_after const firstDay2ndWeek(FirstDayOfWeek);
    boost::gregorian::date begin1stWeek = firstDay2ndWeek.get_date(jan1st);

    int diffDays = (begin1stWeek - jan1st).days();
    if (diffDays >= 4) {
        // Consider the 1st week if the 4th of January falls in that week
        boost::gregorian::first_day_of_the_week_before const firstDay1stWeek(FirstDayOfWeek);
        begin1stWeek = firstDay1stWeek.get_date(jan1st);
    }

    boost::gregorian::date const beginNthWeek = begin1stWeek + boost::gregorian::weeks(week - 1);

//        std::cout << boost::gregorian::to_iso_extended_string(jan1st) << std::endl;
//        std::cout << boost::gregorian::to_iso_extended_string(begin1stWeek) << std::endl;
//        std::cout << "Computed time: " << boost::gregorian::to_iso_extended_string(beginNthWeek) << std::endl;

    time_t ttTime = to_time_t(beginNthWeek);
    return ttTime;
}
*/
inline int GetYearFromDate(time_t ttTime) {
    std::tm tmTime = {};
    std::tm *resTm = gmtime_r(&ttTime, &tmTime);
    return resTm->tm_year + 1900;
}

inline int GetWeekFromDate(time_t ttTime) {
    std::tm tmTime = {};
    std::tm *resTm = gmtime_r(&ttTime, &tmTime);
    std::tm tmTime2 = {};

    const std::string &startOfYear = std::to_string(resTm->tm_year + 1900) + "-01-01";
    time_t ttStartYear = GetTimeFromString(startOfYear);
    std::tm *resTm2 = gmtime_r(&ttStartYear, &tmTime2);
    int weekNo = ((resTm->tm_yday + 6)/7);
    if (resTm->tm_wday < resTm2->tm_wday) {
        ++weekNo;
    }
    return weekNo;

//    boost::gregorian::date bDate = boost::posix_time::from_time_t(ttTime).date();
//    return bDate.week_number();
}


inline bool GetWeekFromDate(const std::string &dateStr, int &retYear, int &retWeek,
                     const std::string &pattern = "%4d-%2d-%2d")
{
    time_t ttTime = GetTimeFromString(dateStr);
    if (ttTime == 0) {
        return false;
    }
    retWeek = GetWeekFromDate(ttTime);
    retYear = GetYearFromDate(ttTime);
//    int day, month, year;
//    if (sscanf(dateStr.c_str(), pattern.c_str(), &year, &month, &day) != 3) {
//        return -1;
//    }
//    std::tm date={};
//    date.tm_year=year-1900;
//    date.tm_mon=month-1;
//    date.tm_mday=day;
//    std::mktime(&date);
//    retYear = year;
//    retWeek = (date.tm_yday-date.tm_wday+7)/7;

    return true;
}


inline time_t GetTimeOffsetFromStartOfYear(int year, int week) {
    boost::gregorian::date harvestEvalStartYearDate(year, 01, 01);
    boost::gregorian::date date = harvestEvalStartYearDate + boost::gregorian::days(week * 7 - 7);
    return to_time_t(date);
}


inline std::string TimeToString(time_t ttTime) {
    std::tm * ptm = std::gmtime(&ttTime);
    char buffer[20];
    std::strftime(buffer, 20, "%Y-%m-%d", ptm);
    return buffer;
}

template <typename T>
inline std::string ValueToString(T value, bool isBool = false)
{
    if (value == NOT_AVAILABLE) {
        return "NA";
    }
    if (value == NR) {
        return "NR";
    }
    if (isBool) {
        return value == true ? "TRUE" : "FALSE";
    }
    if (std::is_same<T, double>::value) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(6) << value;
        return stream.str();
    }
    return std::to_string(value);
}

#endif
