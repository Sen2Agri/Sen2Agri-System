#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MACCSMetadata.hpp"

namespace itk
{
class SEN2CORMetadataReader : public itk::LightObject
{
public:
    typedef SEN2CORMetadataReader Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(SEN2CORMetadataReader, itk::LightObject)

    std::unique_ptr<MACCSFileMetadata> ReadMetadataXml(const TiXmlDocument &doc);
    std::unique_ptr<MACCSFileMetadata> ReadMetadata(const std::string &path);
};
}
