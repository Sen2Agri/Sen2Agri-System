#ifndef TimeSeriesAnalysisUtils_h
#define TimeSeriesAnalysisUtils_h

#include <time.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <gsl/gsl_fit.h>
#include <gsl/gsl_cdf.h>

#include "CommonFunctions.h"

#define NOT_AVAILABLE               -10000
#define NR                          -10001
#define SEC_IN_DAY                   86400          // seconds in day = 24 * 3600
#define SEC_IN_WEEK                  604800         // seconds in week = 7 * 24 * 3600

#define DOUBLE_EPSILON                  0.00000001

boost::gregorian::greg_weekday const FirstDayOfWeek = boost::gregorian::Monday;

template <typename T>
inline bool IsNA(T val) {
    return (val == NOT_AVAILABLE || val == NR);
}

inline bool IsEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return false;
}

inline bool IsLessOrEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return (val1 < val2);
}

inline bool IsGreaterOrEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return (val1 > val2);
}

inline bool IsLess(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return false;    // equal values
    }
    return (val1 < val2);
}

inline bool IsGreater(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return false;    // equal values
    }
    return (val1 > val2);
}

inline boost::gregorian::date GetBoostDateFromString(const std::string &strDate) {
    return boost::gregorian::from_simple_string(strDate);
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

//template <typename T>
//inline std::vector<T> GetLinearFit(const std::vector<T>& xData, const std::vector<T>& yData)
//{
//    T xSum = 0, ySum = 0, xxSum = 0, xySum = 0, slope, intercept;
//    for (long i = 0; i < yData.size(); i++)
//    {
//        xSum += xData[i];
//        ySum += yData[i];
//        xxSum += xData[i] * xData[i];
//        xySum += xData[i] * yData[i];
//    }
//    slope = (yData.size() * xySum - xSum * ySum) / (yData.size() * xxSum - xSum * xSum);
//    intercept = (ySum - slope * xSum) / yData.size();
//    std::vector<T> res;
//    res.push_back(slope);
//    res.push_back(intercept);
//    return res;
//}

template <typename T>
bool ComputePValue(const std::vector<T>& xData, const std::vector<T>& yData, double &pValue) {
    if(xData.size() != yData.size() || xData.size() <= 2) {
        return false;
    }
//    int n = 4;
//    double xData1[4] = { 1970, 1980, 1990, 2000};
//    double yData1[4] = {1.23,   11.432,   14.653, 21.6534};
//    double c0, c1, cov00, cov01, cov11, sumsq;
//    gsl_fit_linear (xData1, 1, yData1, 1, n, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

//    std::cout<<"Coefficients\tEstimate\tStd. Error\tt value\tPr(>|t|)"<<std::endl;

//    double stdev0=sqrt(cov00);
//    double t0=c0/stdev0;
//    double pv0=t0<0?2*(1-gsl_cdf_tdist_P(-t0,n-2)):2*(1-gsl_cdf_tdist_P(t0,n-2));//This is the p-value of the constant term
//    std::cout<<"Intercept\t"<<c0<<"\t"<<stdev0<<"\t"<<t0<<"\t"<<pv0<<std::endl;

//    double stdev1=sqrt(cov11);
//    double t1=c1/stdev1;
//    double pv1=t1<0?2*(1-gsl_cdf_tdist_P(-t1,n-2)):2*(1-gsl_cdf_tdist_P(t1,n-2));//This is the p-value of the linear term
//    std::cout<<"x\t"<<c1<<"\t"<<stdev1<<"\t"<<t1<<"\t"<<pv1<<std::endl;

//    double dl=n-2;//degrees of liberty

//    double ySum = 0;
//    for (long i = 0; i < n; i++) {
//        ySum += yData1[i];
//    }
//    double ym = ySum / n; //Average of vector y

//    double sct = 0; // sct = sum of total squares
//    for (long i = 0; i < n; i++) {
//        sct += pow(yData1[i]-ym,2);
//    }
//    //double sct=pow(yData1[0]-ym,2)+pow(yData1[1]-ym,2)+pow(yData1[2]-ym,2)+pow(yData1[3]-ym,2); // sct = sum of total squares
//    double R2=1-sumsq/sct;
//    std::cout<<"Multiple R-squared: "<<R2<<",    Adjusted R-squared: "<<1-double(n-1)/dl*(1-R2)<<std::endl;
//    double F=R2*dl/(1-R2);
//    double p_value=1-gsl_cdf_fdist_P(F,1,dl);
//    std::cout<<"F-statistic:  "<<F<<" on 1 and "<<n-2<<" DF,  p-value: "<<p_value<<std::endl;


    const double* xData1 = &xData[0];
    const double* yData1 = &yData[0];
    int n = xData.size();
    double c0, c1, cov00, cov01, cov11, sumsq;
    gsl_fit_linear (xData1, 1, yData1, 1, n, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

    //std::cout<<"Coefficients\tEstimate\tStd. Error\tt value\tPr(>|t|)"<<std::endl;

    //double stdev0=sqrt(cov00);
    //double t0=c0/stdev0;
    //double pv0=t0<0?2*(1-gsl_cdf_tdist_P(-t0,n-2)):2*(1-gsl_cdf_tdist_P(t0,n-2));//This is the p-value of the constant term
    //std::cout<<"Intercept\t"<<c0<<"\t"<<stdev0<<"\t"<<t0<<"\t"<<pv0<<std::endl;

    //double stdev1=sqrt(cov11);
    //double t1=c1/stdev1;
    //double pv1=t1<0?2*(1-gsl_cdf_tdist_P(-t1,n-2)):2*(1-gsl_cdf_tdist_P(t1,n-2));//This is the p-value of the linear term
    //std::cout<<"x\t"<<c1<<"\t"<<stdev1<<"\t"<<t1<<"\t"<<pv1<<std::endl;

    double dl=n-2;//degrees of liberty
    double ySum = 0;
    for (long i = 0; i < n; i++) {
        ySum += yData1[i];
    }
    double ym = ySum / n; //Average of vector y
    double sct = 0; // sct = sum of total squares
    for (long i = 0; i < n; i++) {
        sct += pow(yData1[i]-ym,2);
    }
    //double sct=pow(yData1[0]-ym,2)+pow(yData1[1]-ym,2)+pow(yData1[2]-ym,2)+pow(yData1[3]-ym,2); // sct = sum of total squares
    double R2=1-sumsq/sct;
    //std::cout<<"Multiple R-squared: "<<R2<<",    Adjusted R-squared: "<<1-double(n-1)/dl*(1-R2)<<std::endl;
    double F=R2*dl/(1-R2);
    double p_value=1-gsl_cdf_fdist_P(F,1,dl);
    //std::cout<<"F-statistic:  "<<F<<" on 1 and "<<n-2<<" DF,  p-value: "<<p_value<<std::endl;
    pValue = p_value;
    return true;
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
        stream << std::fixed << std::setprecision(7) << value;
        return stream.str();
    }
    return std::to_string(value);
}

inline time_t GetTimeFromString(const std::string &strDate) {
    try {
        boost::gregorian::date const d = boost::gregorian::from_simple_string(strDate);
        return to_time_t(d);
    } catch (...) {
        return 0;
    }
}

inline time_t GetTimeOffsetFromStartOfYear(int year, int week) {
    boost::gregorian::date startYearDate(year, 01, 01);
    boost::gregorian::date date = startYearDate + boost::gregorian::days(week * 7 - 7);
    return to_time_t(date);
}

inline time_t FloorDateToWeekStart(time_t ttTime) {
    if (ttTime == 0) {
        return 0;
    }
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

inline bool isLeap( int year )
{
    if ( year % 4 == 0 )
    {
       if ( year % 100 == 0 && year % 400 != 0 ) {
           return false;
       } else {
           return true;
       }
    }
    return false;
}

inline void GetYearAndWeek( tm TM, int &YYYY, int &WW )                       // Reference: https://en.wikipedia.org/wiki/ISO_8601
{
   YYYY = TM.tm_year + 1900;
   int day = TM.tm_yday;

   int Monday = day - ( TM.tm_wday + 6 ) % 7;                          // Monday this week: may be negative down to 1-6 = -5;
   int MondayYear = 1 + ( Monday + 6 ) % 7;                            // First Monday of the year
   int Monday01 = ( MondayYear > 4 ) ? MondayYear - 7 : MondayYear;    // Monday of week 1: should lie between -2 and 4 inclusive
   WW = 1 + ( Monday - Monday01 ) / 7;                                 // Nominal week ... but see below

   // In ISO-8601 there is no week 0 ... it will be week 52 or 53 of the previous year
   if ( WW == 0 )
   {
      YYYY--;
      WW = 52;
      if ( MondayYear == 3 || MondayYear == 4 || ( isLeap( YYYY ) && MondayYear == 2 ) ) WW = 53;
   }

   // Similar issues at the end of the calendar year
   if ( WW == 53)
   {
      int daysInYear = isLeap( YYYY ) ? 366 : 365;
      if ( daysInYear - Monday < 3 )
      {
         YYYY++;
         WW = 1;
      }
   }
}

inline int GetWeekFromDate(time_t ttTime) {
    std::tm tmTime = {};
    int weekNo;
    std::tm *resTm = gmtime_r(&ttTime, &tmTime);
    if (FirstDayOfWeek == boost::gregorian::Sunday) {
        std::tm tmTime2 = {};

        const std::string &startOfYear = std::to_string(resTm->tm_year + 1900) + "-01-01";
        time_t ttStartYear = GetTimeFromString(startOfYear);
        std::tm *resTm2 = gmtime_r(&ttStartYear, &tmTime2);
        weekNo = ((resTm->tm_yday + 6)/7);
        if (resTm->tm_wday < resTm2->tm_wday) {
            ++weekNo;
        }
        if (resTm->tm_wday == 0) {
            ++weekNo;
        }
    } else {
        int year;
        GetYearAndWeek(*resTm, year, weekNo);
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

#endif
