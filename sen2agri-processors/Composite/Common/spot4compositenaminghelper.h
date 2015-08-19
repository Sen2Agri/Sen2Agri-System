#ifndef SPOT4COMPOSITENAMINGHELPER_H
#define SPOT4COMPOSITENAMINGHELPER_H

#include "compositenaminghelper.h"

#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

class Spot4CompositeNamingHelper : public CompositeNamingHelper
{
public:
    Spot4CompositeNamingHelper();
    const char * GetNameOfClass() { return "Spot4CompositeNamingHelper"; }


protected:
    virtual bool DoLoadMetadata();

    std::string AppendSuffixToTiffFileName(std::string& fileName, const std::string& suffix);
    std::string AppendResolutionSuffix(std::string& fileName);

    std::string DeriveFileNameFromImageFileName(const SPOT4Metadata& spot4Metadata, const std::string& replacement);
    std::string buildFullPath(const std::string& fileName);

    std::string getImageFileName(const SPOT4Metadata& spot4Metadata);
    std::string getAotFileName(const SPOT4Metadata& spot4Metadata);
    std::string getCloudFileName(const SPOT4Metadata& spot4Metadata);
    std::string getWaterFileName(const SPOT4Metadata& spot4Metadata);
    std::string getSnowFileName(const SPOT4Metadata& spot4Metadata);

    std::string getPreProcessedAotFileName(const SPOT4Metadata& spot4Metadata);
    std::string getPreProcessedCloudFileName(const SPOT4Metadata& spot4Metadata);
    std::string getPreProcessedWaterFileName(const SPOT4Metadata& spot4Metadata);
    std::string getPreProcessedSnowFileName(const SPOT4Metadata& spot4Metadata);

    std::string getWeightAotFileName(const SPOT4Metadata& spot4Metadata);
    std::string getWeightCloudFileName(const SPOT4Metadata& spot4Metadata);
    std::string getTotalWeightFileName(const SPOT4Metadata& spot4Metadata);
};

#endif // SPOT4COMPOSITENAMINGHELPER_H
