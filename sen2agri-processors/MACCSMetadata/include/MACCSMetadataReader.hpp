#pragma once

#include "itkObjectFactory.h"

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

    MACCSFileMetadata ReadMetadataXml(const TiXmlDocument &doc);
    MACCSFileMetadata ReadMetadata(const std::string &path);
};
}
