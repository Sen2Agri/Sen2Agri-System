#include "MetadataHelperFactory.h"
#include "Spot4MetadataHelper.h"
#include "MACCSMetadataHelper.h"

MetadataHelperFactory::MetadataHelperFactory()
{
    RegisterHelper(new Spot4MetadataHelper());
    RegisterHelper(new MACCSMetadataHelper());
}

MetadataHelperFactory::~MetadataHelperFactory()
{
    for(size_t i = 0; i<m_registreredHelpers.size(); i++) {
        MetadataHelper* pHelper = m_registreredHelpers[i];
        if(pHelper)
            delete pHelper;
    }
    m_registreredHelpers.clear();
}

void MetadataHelperFactory::RegisterHelper(MetadataHelper *pHelper)
{
    m_registreredHelpers.push_back(pHelper);
}

MetadataHelper* MetadataHelperFactory::GetMetadataHelper(std::string& metadataFileName, int nResolution)
{
    for(size_t i = 0; i < m_registreredHelpers.size(); i++) {
        MetadataHelper* pHelper = m_registreredHelpers[i];
        if(pHelper->LoadMetadataFile(metadataFileName, nResolution))
            return pHelper;
    }
    itkExceptionMacro("Unable to read metadata from " << metadataFileName);

    return NULL;
}

