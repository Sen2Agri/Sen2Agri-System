#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MACCSMetadata.hpp"

namespace itk
{
class MACCSMetadataReader : public itk::LightObject
{
public:
    typedef MACCSMetadataReader Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(MACCSMetadataReader, itk::LightObject)

    std::unique_ptr<MACCSFileMetadata> ReadMetadataXml(const TiXmlDocument &doc);
    std::unique_ptr<MACCSFileMetadata> ReadMetadata(const std::string &path);
};
}
