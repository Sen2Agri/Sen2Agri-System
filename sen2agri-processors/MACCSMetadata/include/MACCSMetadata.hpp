#pragma once

#include <string>
#include <vector>

#include "FluentXML.hpp"

struct MACCSConsumer
{
};

struct MACCSExtension
{
};

struct MACCSHeaderMetadata
{
    std::string SchemaVersion;
    std::string SchemaLocation;
    std::string Type;
    std::string FileName;
    std::string FileDescription;
    std::string Notes;
    std::string Mission;
    std::string FileClass;
    std::string FileType;
    std::string ValidityStart;
    std::string ValidityStop;
    std::string FileVersion;
    std::string SourceSystem;
    std::string Creator;
    std::string CreatorVersion;
    std::string CreationDate;
};

struct MACCSInstanceId
{
    std::string ReferenceProductSemantic;
    std::string ReferenceProductInstance;
    std::string AnnexCode;
};

struct MACCSBand
{
    std::string Id;
    std::string Name;
};

struct MACCSImageInformation
{
    std::string ElementName; //
    std::string Format;
    std::string BinaryEncoding;
    std::string DataType;
    std::string NumberOfSignificantBits;
    std::string NoDataValue;
    std::string VAPNoDataValue;
    std::string VAPQuantificationValue;
    std::string AOTNoDataValue;
    std::string AOTQuantificationValue;
    std::string SizeLines;
    std::string SizeColumns;
    std::string SizeBands;
    std::string ImageCompactingTool;
    std::vector<MACCSBand> Bands;
    std::string SubSamplingFactor;
    std::string SubSamplingFactorLine;
    std::string SubSamplingFactorColumn;
    std::string ValuesUnit;
    std::string QuantificationBitValue; //
    std::string ColorSpace;
    std::string BandsOrder;
};

struct MACCSFileMetadata
{
    MACCSHeaderMetadata Header;
    std::vector<MACCSConsumer> Consumers;
    std::vector<MACCSExtension> Extensions;
    MACCSInstanceId InstanceId;
    std::string ReferenceProductHeaderId;
    std::string AnnexCompleteName;
    MACCSImageInformation ImageInformation;
};
