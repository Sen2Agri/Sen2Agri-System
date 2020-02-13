#ifndef TsaHelperFunctions_H
#define TsaHelperFunctions_H

#include "TimeSeriesAnalysisTypes.h"

template <typename Value>
inline std::vector<Value> FilterValuesByDates(const std::vector<Value> &mergedValues,
                                                  time_t ttStartTime, time_t ttEndTime) {
    // if no filter, return the initial vector
    int vecSize = mergedValues.size();
    if ((ttStartTime == 0 && ttEndTime == 0) || (vecSize == 0)) {
        return mergedValues;
    }
    // assuming they are sorted, check the first and the last
    if (ttStartTime <= mergedValues[0].ttDate &&
            ttEndTime >= mergedValues[vecSize-1].ttDate) {
        return mergedValues;
    }
    // we need to filter values
    std::vector<Value> retVect;
    for (typename std::vector<Value>::const_iterator it = mergedValues.begin(); it != mergedValues.end(); ++it) {
        if(it->ttDate >= ttStartTime && it->ttDate <= ttEndTime) {
            retVect.push_back(*it);
        }
    }
    return retVect;
}

inline bool AllNdviMeanAreNA(const std::vector<EfaMarkersInfoType> &efaMarkers) {
    bool allNdviMeanAreNA = true;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        if (!IsNA(efaMarkers[i].ndviMean)) {
            allNdviMeanAreNA = false;
            break;
        }
    }
    return allNdviMeanAreNA;
}

inline bool CheckCountryCode(const std::string &str, const std::string &countryCode) {
    return boost::starts_with(str, countryCode);
}

inline bool IsNdviPresence(const std::vector<EfaMarkersInfoType> &efaMarkers) {
    bool ndviPresence = false;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        //if (!IsNA(efaMarkers[i].ndviPresence) && efaMarkers[i].ndviPresence == true) {
        if (efaMarkers[i].ndviPresence == true) {
            ndviPresence = true;
            break;
        }
    }
    return ndviPresence;
}

inline bool GetMinMaxNdviValues(const std::vector<MergedAllValInfosType> &values, double &minVal, double &maxVal) {
    bool hasValidNdvi = false;
    double curMinVal = NOT_AVAILABLE;
    double curMaxVal = NOT_AVAILABLE;
    for(size_t i = 0; i<values.size(); i++) {
        if (IsNA(values[i].ndviMeanVal)) {
            continue;
        }
        hasValidNdvi = true;
        if (IsNA(curMinVal)) {
            curMinVal = values[i].ndviMeanVal;
            curMaxVal = curMinVal;
        } else {
            if (IsGreater(curMinVal, values[i].ndviMeanVal)) {
                curMinVal = values[i].ndviMeanVal;
            }
            if (IsLess(curMaxVal, values[i].ndviMeanVal)) {
                curMaxVal = values[i].ndviMeanVal;
            }
        }
    }
    minVal = curMinVal;
    maxVal = curMaxVal;

    return hasValidNdvi;
}

inline bool HasValidNdviValues(const std::vector<MergedAllValInfosType> &values, double thrVal = NOT_AVAILABLE, bool filterVegWeeks = false) {
    bool hasValidNdvi = false;
    for(size_t i = 0; i<values.size(); i++) {
        if (filterVegWeeks && values[i].vegWeeks != true) {
            continue;
        }
        if (IsNA(values[i].ndviMeanVal)) {
            continue;
        }
        if (IsGreater(values[i].ndviMeanVal, thrVal)) {
            hasValidNdvi = true;
            break;
        }
    }
    return hasValidNdvi;
}


#endif
