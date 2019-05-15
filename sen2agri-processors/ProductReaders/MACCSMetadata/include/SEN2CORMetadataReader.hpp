#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MACCSMetadata.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define SEN2COR_METADATA_READER_EXPORT __declspec(dllexport)
#else
#  define SEN2COR_METADATA_READER_EXPORT
#endif

namespace itk
{
class SEN2COR_METADATA_READER_EXPORT SEN2CORMetadataReader : public itk::LightObject
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
