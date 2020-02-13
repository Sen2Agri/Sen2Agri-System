#ifndef TsaCountryPluginsFactory_H
#define TsaCountryPluginsFactory_H

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "TsaCountryDefaultPlugin.h"

#include <vector>
#include <memory>

class TsaCountryPluginsFactory : public itk::LightObject
{
public:
    typedef TsaCountryPluginsFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(TsaCountryPluginsFactory, itk::LightObject)

    inline std::unique_ptr<TsaCountryPluginIntf> GetCountryInfo(const std::string &name) {

        // TODO: Add other countries here too
        std::unique_ptr<TsaCountryPluginIntf> defaultImpl(new TsaCountryDefaultPlugin);
        if (defaultImpl->GetName() == name) {
            return defaultImpl;
        }
    }
};

#endif
