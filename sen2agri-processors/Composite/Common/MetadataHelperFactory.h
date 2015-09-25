#ifndef COMPOSITENAMINGHELPERFACTORY_H
#define COMPOSITENAMINGHELPERFACTORY_H

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "MetadataHelper.h"
#include <vector>
#include <memory>

class MetadataHelperFactory : public itk::LightObject
{
public:
    typedef MetadataHelperFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(MetadataHelperFactory, itk::LightObject)

    std::unique_ptr<MetadataHelper> GetMetadataHelper(const std::string& metadataFileName, int nResolution = 10);
};

#endif // COMPOSITENAMINGHELPERFACTORY_H
