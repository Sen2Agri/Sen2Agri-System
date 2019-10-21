#ifndef CommonFunctions_h
#define CommonFunctions_h

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>

#include "CommonDefs.h"

boost::gregorian::greg_weekday const FirstDayOfWeek = boost::gregorian::Monday;

inline std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (std::getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
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

template <typename T>
inline std::string ValueToString(T value, bool isBool = false)
{
    if (value == NOT_AVAILABLE) {
        return NA_STR;
    }
    if (value == NR) {
        return NR_STR;
    }
    if (value == NOT_AVAILABLE_1) {
        return NA1_STR;
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

inline std::string TimeToString(time_t ttTime) {
    if (ttTime == 0 || ttTime == NOT_AVAILABLE) {
        return NA_STR;
    } else if (ttTime == NR) {
        return NR_STR;
    } else if (ttTime == NOT_AVAILABLE_1) {
        return NA1_STR;
    }
    std::tm * ptm = std::gmtime(&ttTime);
    char buffer[20];
    std::strftime(buffer, 20, "%Y-%m-%d", ptm);
    return buffer;
}

inline std::time_t pt_to_time_t(const boost::posix_time::ptime& pt)
{
    boost::posix_time::ptime timet_start(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration diff = pt - timet_start;
    return diff.ticks()/boost::posix_time::time_duration::rep_type::ticks_per_second;

}

inline time_t GetTimeFromString(const std::string &strDate, const std::string &formatStr) {
    std::locale format = std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(formatStr));
    std::istringstream is(strDate);
    boost::posix_time::ptime pt;
    is.imbue(format);
    is >> pt;
    if(pt != boost::posix_time::ptime())  {
        return pt_to_time_t(pt);
    }
    return 0;
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

inline bool GetWeekFromDate(const std::string &dateStr, int &retYear, int &retWeek)
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

inline std::string trim(std::string const& str, const std::string strChars="\"")
{
    if(str.empty())
        return str;

    int nbChars=strChars.length();
    std::string retStr = str;
    for(int i = 0; i<nbChars; i++) {
        int curChar = strChars.at(i);
        std::size_t firstScan = retStr.find_first_not_of(curChar);
        std::size_t first     = firstScan == std::string::npos ? retStr.length() : firstScan;
        std::size_t last      = retStr.find_last_not_of(curChar);
        retStr = retStr.substr(first, last-first+1);
    }
    return retStr;
}

inline void NormalizeFieldId(std::string &fieldId) {
    std::replace( fieldId.begin(), fieldId.end(), '/', '_');
    fieldId = trim( fieldId);
}

inline bool GetFileInfosFromName(const std::string &filePath, std::string &fileType, std::string & polarisation,
                          std::string & orbit, time_t &fileDate, time_t &additionalFileDate, bool useLatestNameFormat = true)
{
    boost::filesystem::path p(filePath);
    boost::filesystem::path pf = p.filename();
    std::string fileName = pf.string();

    std::string lowerCaseFileName = fileName;
    boost::algorithm::to_lower(lowerCaseFileName);
    std::string lowerCaseAmpStr = AMP_STR;
    std::string lowerCaseCoheStr = COHE_STR;
    boost::algorithm::to_lower(lowerCaseAmpStr);
    boost::algorithm::to_lower(lowerCaseCoheStr);

    std::string regexExpStr;
    int dateRegexIdx;
    int dateRegexIdx2 = -1;
    int orbitIdx = -1;
    additionalFileDate = 0;
    bool useDate2 = false;

    if (fileName.find(NDVI_STR) != std::string::npos) {
        fileType = NDVI_FT;
        regexExpStr = NDVI_REGEX;
        dateRegexIdx = NDVI_REGEX_DATE_IDX;
    } else if (fileName.find(VV_STR) != std::string::npos) {
        polarisation = "VV";
        if (lowerCaseFileName.find(lowerCaseAmpStr) != std::string::npos) {
            fileType = AMP_FT;
            regexExpStr = useLatestNameFormat ? AMP_VV_REGEX : AMP_VV_REGEX_OLD;
            dateRegexIdx = AMP_REGEX_DATE_IDX;
            dateRegexIdx2 = useLatestNameFormat ? AMP_REGEX_DATE2_IDX : -1; // we did not had 2 dates here in 2017
        } else if (lowerCaseFileName.find(lowerCaseCoheStr) != std::string::npos) {
            fileType = COHE_FT;
            useDate2 = true;
            regexExpStr = useLatestNameFormat ? COHE_VV_REGEX : COHE_VV_REGEX_OLD;
            dateRegexIdx = COHE_REGEX_DATE_IDX;
            dateRegexIdx2 = COHE_REGEX_DATE2_IDX;
            orbitIdx = COHE_REGEX_ORBIT_IDX;
        }
    } else if (fileName.find(VH_STR) != std::string::npos) {
        polarisation = "VH";
        if (lowerCaseFileName.find(lowerCaseAmpStr) != std::string::npos) {
            fileType = AMP_FT;
            regexExpStr = useLatestNameFormat ? AMP_VH_REGEX : AMP_VH_REGEX_OLD;
            dateRegexIdx = AMP_REGEX_DATE_IDX;
            dateRegexIdx2 = useLatestNameFormat ? AMP_REGEX_DATE2_IDX : -1; // we did not had 2 dates here in 2017
        } else if (lowerCaseFileName.find(lowerCaseCoheStr) != std::string::npos) {
            fileType = COHE_FT;
            useDate2 = true;
            regexExpStr = useLatestNameFormat ? COHE_VH_REGEX : COHE_VH_REGEX_OLD;
            dateRegexIdx = COHE_REGEX_DATE_IDX;
            dateRegexIdx2 = COHE_REGEX_DATE2_IDX;
            orbitIdx = COHE_REGEX_ORBIT_IDX;
        }
    } else if (fileName.find(LAI_STR) != std::string::npos) {
        fileType = "LAI";
        regexExpStr = LAI_REGEX;
        dateRegexIdx = LAI_REGEX_DATE_IDX;
    } else {
        return false;
    }

    boost::regex regexExp {regexExpStr};
    boost::smatch matches;
    if (boost::regex_match(fileName,matches,regexExp)) {
        const std::string &fileDateTmp = matches[dateRegexIdx].str();
        fileDate = to_time_t(boost::gregorian::from_undelimited_string(fileDateTmp));

        if (dateRegexIdx2 != -1) {
            const std::string &fileDateTmp2 = matches[dateRegexIdx2].str();
            time_t fileDate2 = to_time_t(boost::gregorian::from_undelimited_string(fileDateTmp2));
            // if we have 2 dates, get the maximum of the dates as we do not know the order
            additionalFileDate = fileDate2;
            if (fileDate < fileDate2) {
                additionalFileDate = fileDate;
                fileDate = fileDate2;
            }
            // even if we have 2 dates, for amplitude for ex. we need only one
            if (!useDate2) {
                additionalFileDate = 0;
            }
        }
        if (orbitIdx != -1) {
            orbit = matches[orbitIdx].str();
        }
        return true;
    }

    return false;
}

inline std::string BuildOutputFileName(const std::string &fid, const std::string &fileType,
                                const std::string &polarisation, const std::string &orbitId,
                                time_t ttFileDate, time_t ttAdditionalFileDate, bool fullFileName=false) {
    // 31.0000001696738.001_timeseries_VH - AMP
    // 31.0000001696738.001_SERIES_088_6_day_timeseries_VH.txt
    // 31.0000001696738.001_SERIES_088_12_day_timeseries_VH.txt
    // 31.0000002518067.001_timeseries_NDVI.txt

    std::string fileNameAdditionalField;
    if (fileType == COHE_FT) {
        std::string dateStr = "12_day";
        if ((ttFileDate - ttAdditionalFileDate) == 6 * SEC_IN_DAY) {
            dateStr = "6_day";
        }
        if (fullFileName) {
            fileNameAdditionalField = "SERIES_" + orbitId + "_" + dateStr;      // To be cf. with ATBD
        } else {
            fileNameAdditionalField = orbitId + "_" + dateStr;      // To be cf. with ATBD
        }
    }
    if (fullFileName) {
        return fid + (fileNameAdditionalField.size() > 0 ? "_" + fileNameAdditionalField : "") +
                "_timeseries_" + (polarisation.size() > 0 ? polarisation : fileType) + ".txt";
    } else {
        return (fileNameAdditionalField.size() > 0 ? (fileNameAdditionalField + "_") : "") +
                (polarisation.size() > 0 ? polarisation : fileType);
    }
}

inline std::string GetIndividualFieldFileName(const std::string &outDirPath, const std::string &fileName) {
    boost::filesystem::path rootFolder(outDirPath);
    // check if this path is a folder or a file
    // if it is a file, then we get its parent folder
    if (!boost::filesystem::is_directory(rootFolder)) {
        rootFolder = rootFolder.parent_path();
    }
    return (rootFolder / fileName).string() + ".txt";
}


inline std::string DoubleToString(double value, int precission = 7)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precission) << value;
    return stream.str();
}

#endif
