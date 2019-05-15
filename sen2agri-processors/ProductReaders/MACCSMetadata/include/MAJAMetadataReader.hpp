#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MAJAMetadata.hpp"
#include "MACCSMetadata.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define MAJA_METADATA_READER_EXPORT __declspec(dllexport)
#else
#  define MAJA_METADATA_READER_EXPORT
#endif

namespace itk
{
class MAJA_METADATA_READER_EXPORT MAJAMetadataReader : public itk::LightObject
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
