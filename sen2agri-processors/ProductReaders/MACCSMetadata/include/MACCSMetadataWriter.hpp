#pragma once

#include "itkObjectFactory.h"

#include "FluentXML.hpp"
#include "MACCSMetadata.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define MACCS_METADATA_WRITER_EXPORT __declspec(dllexport)
#else
#  define MACCS_METADATA_WRITER_EXPORT
#endif

namespace itk
{
class MACCS_METADATA_WRITER_EXPORT MACCSMetadataWriter : public itk::LightObject
{
public:
    typedef MACCSMetadataWriter Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(MACCSMetadataWriter, itk::LightObject)

    XDocument CreateMetadataXml(const MACCSFileMetadata &metadata);
    void WriteMetadata(const MACCSFileMetadata &metadata, const std::string &path);
};
}
