#ifndef COMPOSITENAMINGHELPERFACTORY_H
#define COMPOSITENAMINGHELPERFACTORY_H


#include "compositenaminghelper.h"
#include <vector>

class CompositeNamingHelperFactory
{
public:
    ~CompositeNamingHelperFactory();
    CompositeNamingHelper* GetNamingHelper(std::string& metadataFileName, int nRes);
    static CompositeNamingHelperFactory *GetInstance()
    {
            static CompositeNamingHelperFactory instance;
            return &instance;
    }

    const char * GetNameOfClass() { return "CompositeNamingHelperFactory";}
private:
    CompositeNamingHelperFactory();
    void RegisterHelper(CompositeNamingHelper *pHelper);
    std::vector<CompositeNamingHelper*> m_registreredHelpers;
};

#endif // COMPOSITENAMINGHELPERFACTORY_H
