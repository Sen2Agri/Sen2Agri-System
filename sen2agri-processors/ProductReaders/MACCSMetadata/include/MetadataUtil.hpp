#include <string>

#include "MACCSMetadata.hpp"
#include "SPOT4Metadata.hpp"

std::string getRasterFile(const MACCSFileMetadata &metadata, const char *suffix);
std::string getRasterFile(const SPOT4Metadata &metadata, const std::string &file);

std::string getMainRasterFile(const MACCSFileMetadata &metadata);
std::string getMainRasterFile(const SPOT4Metadata &metadata);

std::string getMainRasterFile(const std::string &path);
