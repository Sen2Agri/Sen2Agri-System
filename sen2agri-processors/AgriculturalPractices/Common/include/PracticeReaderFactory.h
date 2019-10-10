#ifndef PracticeReaderFactory_h
#define PracticeReaderFactory_h

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "PracticeReaderBase.h"

class PracticeReaderFactory : public itk::LightObject
{
public:
    typedef PracticeReaderFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(PracticeReaderFactory, itk::LightObject)

    std::unique_ptr<PracticeReaderBase> GetPracticeReader(const std::string &name);
};

#endif
