#pragma once

#include <memory>

#include "itkObjectFactory.h"

#include "SPOT4Metadata.hpp"
#include "otb_tinyxml.h"

namespace itk
{
class SPOT4MetadataReader : public itk::LightObject
{
public:
    typedef SPOT4MetadataReader Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(SPOT4MetadataReader, itk::LightObject)

    std::unique_ptr<SPOT4Metadata> ReadMetadataXml(const TiXmlDocument &doc);
    std::unique_ptr<SPOT4Metadata> ReadMetadata(const std::string &path);
};
}
