#include "compositenaminghelperfactory.h"
#include "spot4compositenaminghelper.h"

CompositeNamingHelperFactory::CompositeNamingHelperFactory()
{
    RegisterHelper(new Spot4CompositeNamingHelper());
}

CompositeNamingHelperFactory::~CompositeNamingHelperFactory()
{
    for(size_t i = 0; i<m_registreredHelpers.size(); i++) {
        CompositeNamingHelper* pHelper = m_registreredHelpers.at(i);
        if(pHelper)
            delete pHelper;
    }
    m_registreredHelpers.clear();
}

void CompositeNamingHelperFactory::RegisterHelper(CompositeNamingHelper *pHelper)
{
    m_registreredHelpers.push_back(pHelper);
}

CompositeNamingHelper* CompositeNamingHelperFactory::GetNamingHelper(std::string& metadataFileName, int nRes)
{
    for(size_t i = 0; i < m_registreredHelpers.size(); i++) {
        CompositeNamingHelper* pHelper = m_registreredHelpers.at(i);
        if(pHelper->LoadMetadataFile(metadataFileName, nRes))
            return pHelper;
    }
    itkExceptionMacro("Unable to read metadata from " << metadataFileName);

    return NULL;
}

