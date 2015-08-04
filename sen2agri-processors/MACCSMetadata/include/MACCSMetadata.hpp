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

struct MACCSFixedHeader
{
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

struct MACCSMainProductHeader
{
    std::vector<MACCSConsumer> Consumers;
    std::vector<MACCSExtension> Extensions;
};

struct MACCSHeaderMetadata
{
    std::string SchemaVersion;
    std::string SchemaLocation;
    std::string Type;
    MACCSFixedHeader FixedHeader;
};

struct MACCSInstanceId
{
    std::string ReferenceProductSemantic;
    std::string ReferenceProductInstance;
    std::string AnnexCode;
    std::string NickName;
    std::string AcquisitionDate;
};

struct MACCSSize
{
    std::string Lines;
    std::string Columns;
    std::string Bands;
};

struct MACCSBand
{
    std::string Id;
    std::string Name;
};

struct MACCSGeoPosition
{
    std::string UnitLengthX;
    std::string UnitLengthY;
    std::string DimensionX;
    std::string DimensionY;
};

struct MACCSProductSampling
{
    std::string ByLineUnit;
    std::string ByLineValue;
    std::string ByColumnUnit;
    std::string ByColumnValue;
};

struct MACCSResolution
{
    std::string Id;
    MACCSSize Size;
    MACCSGeoPosition GeoPosition;
    MACCSProductSampling ProductSampling;
    std::vector<MACCSBand> Bands;
};

struct MACCSImageInformation
{
    std::string ElementName;
    std::string Format;
    std::string BinaryEncoding;
    std::string DataType;
    std::string NumberOfSignificantBits;
    std::string NoDataValue;
    std::string VAPNoDataValue;
    std::string VAPQuantificationValue;
    std::string AOTNoDataValue;
    std::string AOTQuantificationValue;
    MACCSSize Size;
    std::string ImageCompactingTool;
    std::vector<MACCSResolution> Resolutions;
    std::vector<MACCSBand> Bands;
    std::string SubSamplingFactor;
    std::string SubSamplingFactorLine;
    std::string SubSamplingFactorColumn;
    std::string ValuesUnit;
    std::string QuantificationBitValue;
    std::string ColorSpace;
    std::string BandsOrder;
};

struct MACCSFileInformation
{
    std::string Nature;
    std::string FileLocation;
    std::string LogicalName;
};

struct MACCSAnnexInformation
{
    std::string Id;
    MACCSFileInformation File;
};

struct MACCSProductOrganization
{
    std::vector<MACCSFileInformation> ImageFiles;
    std::vector<MACCSFileInformation> QuickLookFiles;
    std::vector<MACCSAnnexInformation> AnnexFiles;
};

struct MACCSFileMetadata
{
    MACCSHeaderMetadata Header;
    MACCSMainProductHeader MainProductHeader;
    MACCSInstanceId InstanceId;
    std::string ReferenceProductHeaderId;
    std::string AnnexCompleteName;
    MACCSImageInformation ImageInformation;
    MACCSProductOrganization ProductOrganization;
};
