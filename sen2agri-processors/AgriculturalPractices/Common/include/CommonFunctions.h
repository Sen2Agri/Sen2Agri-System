#ifndef CommonFunctions_h
#define CommonFunctions_h

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "CommonDefs.h"

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

inline std::string TimeToString(time_t ttTime) {
    if (ttTime == 0 || ttTime == NOT_AVAILABLE) {
        return "NA";
    } else if (ttTime == NR) {
        return "NR";
    }
    std::tm * ptm = std::gmtime(&ttTime);
    char buffer[20];
    std::strftime(buffer, 20, "%Y-%m-%d", ptm);
    return buffer;
}


inline void NormalizeFieldId(std::string &fieldId) {
    std::replace( fieldId.begin(), fieldId.end(), '/', '_');
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
