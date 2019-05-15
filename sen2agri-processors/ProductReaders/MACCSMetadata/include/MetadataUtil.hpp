#include <string>

#include "CommonMetadata.hpp"
#include "MACCSMetadata.hpp"
#include "SPOT4Metadata.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define METADATA_UTIL_EXPORT __declspec(dllexport)
#else
#  define METADATA_UTIL_EXPORT
#endif

std::string METADATA_UTIL_EXPORT getRasterFile(const MACCSFileMetadata &metadata, const char *suffix);
std::string METADATA_UTIL_EXPORT getRasterFile(const SPOT4Metadata &metadata, const std::string &file);

std::string METADATA_UTIL_EXPORT getMainRasterFile(const MACCSFileMetadata &metadata);
std::string METADATA_UTIL_EXPORT getMainRasterFile(const SPOT4Metadata &metadata);

std::string METADATA_UTIL_EXPORT getMainRasterFile(const std::string &path);

CommonAnglePair METADATA_UTIL_EXPORT ReadAnglePair(const TiXmlElement *el, const std::string &zenithElName,
                             const std::string &azimuthElName);
CommonMeanViewingIncidenceAngle METADATA_UTIL_EXPORT ReadMeanViewingIncidenceAngle(const TiXmlElement *el);
std::vector<CommonMeanViewingIncidenceAngle> METADATA_UTIL_EXPORT ReadMeanViewingIncidenceAngles(const TiXmlElement *el);
CommonAngles METADATA_UTIL_EXPORT ReadSolarAngles(const TiXmlElement *el);
CommonGeoPosition METADATA_UTIL_EXPORT ReadGeoPosition(const TiXmlElement *el);
std::vector<CommonViewingAnglesGrid> METADATA_UTIL_EXPORT ReadViewingAnglesGridList(const TiXmlElement *el);
std::string METADATA_UTIL_EXPORT ExtractDateFromDateTime(std::string dateTime);
std::string METADATA_UTIL_EXPORT GetLogicalFileName(std::string filePath, bool withExtension = true);

