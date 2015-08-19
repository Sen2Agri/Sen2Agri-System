#ifndef SPOT4METADATAHELPER_H
#define SPOT4METADATAHELPER_H

#include "MetadataHelper.h"

#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;


class Spot4MetadataHelper : public MetadataHelper
{
public:
    Spot4MetadataHelper();

    const char * GetNameOfClass() { return "Spot4MetadataHelper"; }

protected:
    virtual bool DoLoadMetadata();

    std::string DeriveFileNameFromImageFileName(const SPOT4Metadata& spot4Metadata, const std::string& replacement);
    std::string buildFullPath(const std::string& fileName);

    std::string getImageFileName(const SPOT4Metadata& spot4Metadata);
    std::string getAotFileName(const SPOT4Metadata& spot4Metadata);
    std::string getCloudFileName(const SPOT4Metadata& spot4Metadata);
    std::string getWaterFileName(const SPOT4Metadata& spot4Metadata);
    std::string getSnowFileName(const SPOT4Metadata& spot4Metadata);
};

#endif // SPOT4METADATAHELPER_H
