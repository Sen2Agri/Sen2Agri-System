#include "MetadataHelperFactory.h"
#include "Spot4MetadataHelper.h"
#include "MACCSMetadataHelper.h"

std::unique_ptr<MetadataHelper> MetadataHelperFactory::GetMetadataHelper(std::string& metadataFileName, int nResolution)
{
    std::unique_ptr<MetadataHelper> spot4MetadataHelper(new Spot4MetadataHelper);
    if (spot4MetadataHelper->LoadMetadataFile(metadataFileName, nResolution))
        return spot4MetadataHelper;

    std::unique_ptr<MetadataHelper> maccsMetadataHelper(new MACCSMetadataHelper);
    if (maccsMetadataHelper->LoadMetadataFile(metadataFileName, nResolution))
        return maccsMetadataHelper;

    itkExceptionMacro("Unable to read metadata from " << metadataFileName);

    return NULL;
}
