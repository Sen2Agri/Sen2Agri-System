#pragma once

#include "itkObjectFactory.h"

#include "FluentXML.hpp"
#include "TileMetadata.hpp"

namespace itk
{
class TileMetadataWriter : public itk::LightObject
{
public:
    typedef TileMetadataWriter Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

public:
    itkNewMacro(Self)

    itkTypeMacro(TileMetadataWriter, itk::LightObject)

    XDocument CreateTileMetadataXml(const TileFileMetadata &metadata);
    void WriteTileMetadata(const TileFileMetadata &metadata, const std::string &path);
};
}
