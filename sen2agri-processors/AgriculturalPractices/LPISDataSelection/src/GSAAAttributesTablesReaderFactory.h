#ifndef GSAAAttributesTablesReaderFactory_h
#define GSAAAttributesTablesReaderFactory_h

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "GSAAAttributesTablesReaderBase.h"

class GSAAAttributesTablesReaderFactory : public itk::LightObject
{
public:
    typedef GSAAAttributesTablesReaderFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(GSAAAttributesTablesReaderFactory, itk::LightObject)

    std::unique_ptr<GSAAAttributesTablesReaderBase> GetPracticeReader(const std::string &name);
};

#endif
