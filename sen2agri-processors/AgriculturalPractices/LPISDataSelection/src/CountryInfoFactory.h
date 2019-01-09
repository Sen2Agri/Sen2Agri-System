#ifndef CountryInfoFactory_H
#define CountryInfoFactory_H

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "CzeCountryInfo.h"
#include "EspCountryInfo.h"
#include "ItaCountryInfo.h"
#include "LtuCountryInfo.h"
#include "NlCountryInfo.h"
#include "RouCountryInfo.h"

#include <vector>
#include <memory>

class CountryInfoFactory : public itk::LightObject
{
public:
    typedef CountryInfoFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(CountryInfoFactory, itk::LightObject)

    inline std::unique_ptr<CountryInfoBase> GetCountryInfo(const std::string &name) {
        std::unique_ptr<CountryInfoBase> czeInfos(new CzeCountryInfo);
        if (czeInfos->GetName() == name) {
            return czeInfos;
        }
        std::unique_ptr<CountryInfoBase> nlInfos(new NlCountryInfo);
        if (nlInfos->GetName() == name) {
            return nlInfos;
        }
        std::unique_ptr<CountryInfoBase> ltuInfos(new LtuCountryInfo);
        if (ltuInfos->GetName() == name) {
            return ltuInfos;
        }
        std::unique_ptr<CountryInfoBase> rouInfos(new RouCountryInfo);
        if (rouInfos->GetName() == name) {
            return rouInfos;
        }
        std::unique_ptr<CountryInfoBase> itaInfos(new ItaCountryInfo);
        if (itaInfos->GetName() == name) {
            return itaInfos;
        }
        std::unique_ptr<CountryInfoBase> espInfos(new EspCountryInfo);
        if (espInfos->GetName() == name) {
            return espInfos;
        }

        itkExceptionMacro("Practice reader not supported: " << name);

        return NULL;
    }
};

#endif
