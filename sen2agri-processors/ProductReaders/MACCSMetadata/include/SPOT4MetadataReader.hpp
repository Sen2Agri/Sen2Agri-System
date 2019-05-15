#pragma once

#include <memory>

#include "itkObjectFactory.h"

#include "SPOT4Metadata.hpp"
#include "otb_tinyxml.h"

#if (defined(WIN32) || defined(_WIN32))
#  define SPOT4_METADATA_READER_EXPORT __declspec(dllexport)
#else
#  define SPOT4_METADATA_READER_EXPORT
#endif

namespace itk
{
class SPOT4_METADATA_READER_EXPORT SPOT4MetadataReader : public itk::LightObject
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
