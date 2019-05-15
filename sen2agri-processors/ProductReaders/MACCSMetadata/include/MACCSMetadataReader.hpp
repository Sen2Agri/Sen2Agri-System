#pragma once

#include <memory>

#include "itkObjectFactory.h"
#include "otb_tinyxml.h"

#include "MACCSMetadata.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define MACCS_METADATA_READER_EXPORT __declspec(dllexport)
#else
#  define MACCS_METADATA_READER_EXPORT
#endif

namespace itk
{
class MACCS_METADATA_READER_EXPORT MACCSMetadataReader : public itk::LightObject
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
