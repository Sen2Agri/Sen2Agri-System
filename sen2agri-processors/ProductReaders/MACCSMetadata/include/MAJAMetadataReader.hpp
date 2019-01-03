#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MAJAMetadata.hpp"
#include "MACCSMetadata.hpp"

namespace itk
{
class MAJAMetadataReader : public itk::LightObject
{
public:
    typedef MAJAMetadataReader Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(MAJAMetadataReader, itk::LightObject)

    std::unique_ptr<MACCSFileMetadata> ReadMetadataXml(const TiXmlDocument &doc);
    std::unique_ptr<MACCSFileMetadata> ReadMetadata(const std::string &path);
};
}
