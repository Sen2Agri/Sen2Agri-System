#include <string>

#include "CommonMetadata.hpp"
#include "MACCSMetadata.hpp"
#include "SPOT4Metadata.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

std::string getRasterFile(const MACCSFileMetadata &metadata, const char *suffix);
std::string getRasterFile(const SPOT4Metadata &metadata, const std::string &file);

std::string getMainRasterFile(const MACCSFileMetadata &metadata);
std::string getMainRasterFile(const SPOT4Metadata &metadata);

std::string getMainRasterFile(const std::string &path);

CommonAnglePair ReadAnglePair(const TiXmlElement *el, const std::string &zenithElName,
                             const std::string &azimuthElName);
CommonMeanViewingIncidenceAngle ReadMeanViewingIncidenceAngle(const TiXmlElement *el);
std::vector<CommonMeanViewingIncidenceAngle> ReadMeanViewingIncidenceAngles(const TiXmlElement *el);
CommonAngles ReadSolarAngles(const TiXmlElement *el);
CommonGeoPosition ReadGeoPosition(const TiXmlElement *el);
std::vector<CommonViewingAnglesGrid> ReadViewingAnglesGridList(const TiXmlElement *el);
std::string ExtractDateFromDateTime(std::string dateTime);
std::string GetLogicalFileName(std::string filePath, bool withExtension = true);

