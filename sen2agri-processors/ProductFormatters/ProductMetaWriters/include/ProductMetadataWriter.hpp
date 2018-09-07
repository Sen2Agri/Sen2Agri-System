#pragma once

#include "itkObjectFactory.h"

#include "FluentXML.hpp"
#include "ProductMetadata.hpp"

namespace itk
{
class ProductMetadataWriter : public itk::LightObject
{
public:
    typedef ProductMetadataWriter Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(ProductMetadataWriter, itk::LightObject)

    XDocument CreateProductMetadataXml(const ProductFileMetadata &metadata);
    void WriteProductMetadata(const ProductFileMetadata &metadata, const std::string &path);
};
}
