#ifndef COMPOSITENAMINGHELPERFACTORY_H
#define COMPOSITENAMINGHELPERFACTORY_H


#include "MetadataHelper.h"
#include <vector>

class MetadataHelperFactory
{
public:
    ~MetadataHelperFactory();
    MetadataHelper *GetMetadataHelper(std::string& metadataFileName, int nResolution);
    static MetadataHelperFactory *GetInstance()
    {
            static MetadataHelperFactory instance;
            return &instance;
    }

    const char * GetNameOfClass() { return "MetadataHelperFactory";}
private:
    MetadataHelperFactory();
    void RegisterHelper(MetadataHelper *pHelper);
    std::vector<MetadataHelper*> m_registreredHelpers;
};

#endif // COMPOSITENAMINGHELPERFACTORY_H
