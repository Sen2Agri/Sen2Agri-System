#ifndef DataSourceFactory_H
#define DataSourceFactory_H

#include "itkLightObject.h"
#include "itkObjectFactory.h"
#include <vector>
#include <memory>


#include "DataSource.h"

class DataSourceFactory : public itk::LightObject
{
public:
    typedef DataSourceFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(DataSourceFactory, itk::LightObject)

    inline std::unique_ptr<DataSource> GetDataSource(const std::string &name) {
//        std::unique_ptr<CountryInfoBase> czeInfos(new CzeCountryInfo);
//        if (czeInfos->GetName() == name) {
//            return czeInfos;
//        }
//        std::unique_ptr<CountryInfoBase> nlInfos(new NlCountryInfo);
//        if (nlInfos->GetName() == name) {
//            return nlInfos;
//        }
//        std::unique_ptr<CountryInfoBase> ltuInfos(new LtuCountryInfo);
//        if (ltuInfos->GetName() == name) {
//            return ltuInfos;
//        }
//        std::unique_ptr<CountryInfoBase> rouInfos(new RouCountryInfo);
//        if (rouInfos->GetName() == name) {
//            return rouInfos;
//        }
//        std::unique_ptr<CountryInfoBase> itaInfos(new ItaCountryInfo);
//        if (itaInfos->GetName() == name) {
//            return itaInfos;
//        }
//        std::unique_ptr<CountryInfoBase> espInfos(new EspCountryInfo);
//        if (espInfos->GetName() == name) {
//            return espInfos;
//        }
//
//        itkExceptionMacro("Practice reader not supported: " << name);

        return NULL;
    }
};


#endif
