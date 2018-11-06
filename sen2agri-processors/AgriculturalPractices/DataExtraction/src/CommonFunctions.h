#ifndef CommonFunctions_h
#define CommonFunctions_h

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define SEC_IN_DAY                   86400          // seconds in day = 24 * 3600

#define NDVI_STR    "_SNDVI_"
#define VV_STR      "_VV_"
#define VH_STR      "_VH_"
#define LAI_STR     "_SLAI_"

#define AMP_STR      "_AMP"
#define COHE_STR     "_COHE"


#define NDVI_REGEX          R"(S2AGRI_L3B_SNDVI_A(\d{8})T.*\.TIF)"
#define LAI_REGEX           R"(S2AGRI_L3B_SLAI_A(\d{8})T.*\.TIF)"

// 2017 naming format for coherence and amplitude
#define COHE_VV_REGEX_OLD       R"((\d{8})-(\d{8})_.*_(\d{3})_VV_.*\.tiff)"
#define COHE_VH_REGEX_OLD       R"((\d{8})-(\d{8})_.*_(\d{3})_VH_.*\.tiff)"
#define AMP_VV_REGEX_OLD        R"((\d{8})_.*_VV_.*\.tiff)"
#define AMP_VH_REGEX_OLD        R"((\d{8})_.*_VH_.*\.tiff)"

// 2018 naming format for coherence and amplitude
#define COHE_VV_REGEX       R"(SEN4CAP_L2A_PRD_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VV_(\d{3})_COHE\.tif)"
#define COHE_VH_REGEX       R"(SEN4CAP_L2A_PRD_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VH_(\d{3})_COHE\.tif)"
#define AMP_VV_REGEX        R"(SEN4CAP_L2A_PRD_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VV_\d{3}_AMP\.tif)"
#define AMP_VH_REGEX        R"(SEN4CAP_L2A_PRD_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VH_\d{3}_AMP\.tif)"

#define NDVI_REGEX_DATE_IDX         1

#define COHE_REGEX_DATE_IDX         1           // this is the same for 2017 and 2018 formats
#define COHE_REGEX_DATE2_IDX        2           // this is the same for 2017 and 2018 formats

#define AMP_REGEX_DATE_IDX          1
#define AMP_REGEX_DATE2_IDX         2           // this does not exists for 2017

#define COHE_REGEX_ORBIT_IDX        3           // this is the same for 2017 and 2018 formats

#define LAI_REGEX_DATE_IDX          1

#define NDVI_FT         "NDVI"
#define AMP_FT          "AMP"
#define COHE_FT         "COHE"

std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (std::getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}


void NormalizeFieldId(std::string &fieldId) {
    std::replace( fieldId.begin(), fieldId.end(), '/', '_');
}

time_t to_time_t(const boost::gregorian::date& date ){
    if (date.is_not_a_date()) {
        return 0;
    }

    using namespace boost::posix_time;
    static ptime epoch(boost::gregorian::date(1970, 1, 1));
    time_duration::sec_type secs = (ptime(date,seconds(0)) - epoch).total_seconds();
    return time_t(secs);
}

std::string TimeToString(time_t ttTime) {
    std::tm * ptm = std::gmtime(&ttTime);
    char buffer[20];
    std::strftime(buffer, 20, "%Y-%m-%d", ptm);
    return buffer;
}

bool GetFileInfosFromName(const std::string &filePath, std::string &fileType, std::string & polarisation,
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

std::string BuildOutputFileName(const std::string &fid, const std::string &fileType,
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

std::string DoubleToString(double value, int precission = 7)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precission) << value;
    return stream.str();
}


#endif
