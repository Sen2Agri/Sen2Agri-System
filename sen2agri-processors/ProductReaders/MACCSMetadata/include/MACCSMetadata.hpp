#pragma once

#include <string>
#include <vector>
#include "CommonMetadata.hpp"

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

struct MACCSGeoPoint
{
    std::string UnitLong;
    std::string UnitLat;
    double Long;
    double Lat;
};

struct MACCSGeoCoverage
{
    MACCSGeoPoint UpperLeftCorner;
    MACCSGeoPoint UpperRightCorner;
    MACCSGeoPoint LowerLeftCorner;
    MACCSGeoPoint LowerRightCorner;
};

struct MACCSBandViewingAnglesGrid
{
    std::string BandId;
    CommonAngles Angles;
};

struct MACCSProductInformation
{
    std::string AcquisitionDateTime;
    MACCSGeoCoverage GeoCoverage;
    CommonAnglePair MeanSunAngle;
    CommonAngles SolarAngles;
    std::vector<CommonMeanViewingIncidenceAngle> MeanViewingIncidenceAngles;
    std::vector<CommonViewingAnglesGrid> ViewingAngles;
    std::string ReflectanceQuantificationValue;
    std::vector<CommonBandWavelength> BandWavelengths;
    std::vector<CommonBandResolution> BandResolutions;
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
    CommonSize Size;
    std::string ImageCompactingTool;
    std::vector<CommonResolution> Resolutions;
    std::vector<CommonBand> Bands;
    std::string SubSamplingFactor;
    std::string SubSamplingFactorLine;
    std::string SubSamplingFactorColumn;
    std::string ValuesUnit;
    std::string QuantificationBitValue;
    std::string ColorSpace;
    std::string BandsOrder;
};

struct MACCSFileMetadata
{
    MACCSHeaderMetadata Header;
    MACCSMainProductHeader MainProductHeader;
    MACCSInstanceId InstanceId;
    std::string ReferenceProductHeaderId;
    std::string AnnexCompleteName;
    MACCSProductInformation ProductInformation;
    MACCSImageInformation ImageInformation;
    CommonProductOrganization ProductOrganization;

    std::string ProductPath;
};
