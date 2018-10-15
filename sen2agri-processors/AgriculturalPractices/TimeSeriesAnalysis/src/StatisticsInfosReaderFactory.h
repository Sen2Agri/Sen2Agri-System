#ifndef StatisticsInfosReaderFactory_h
#define StatisticsInfosReaderFactory_h

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "StatisticsInfosReaderBase.h"

class StatisticsInfosReaderFactory : public itk::LightObject
{
public:
    typedef StatisticsInfosReaderFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(StatisticsInfosReaderFactory, itk::LightObject)

    std::unique_ptr<StatisticsInfosReaderBase> GetInfosReader(const std::string &name);
};

#endif
