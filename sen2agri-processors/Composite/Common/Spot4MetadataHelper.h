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

    virtual std::string GetBandName(unsigned int nIdx, bool bRelativeIdx=true);
    // for Spot we have only one resolution
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx) { return nAbsBandIdx; }

protected:
    virtual bool DoLoadMetadata();

    std::string DeriveFileNameFromImageFileName(const std::string& replacement);

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();

    std::unique_ptr<SPOT4Metadata> m_metadata;
};

#endif // SPOT4METADATAHELPER_H
