#pragma once

#include <string>
#include <vector>

struct CommonFileInformation
{
    std::string Nature;
    std::string FileLocation;
    std::string LogicalName;
    std::string BandId;
    std::string GroupId;
    std::int32_t BandNumber;
    std::int32_t BitNumber;
};

struct CommonAnnexInformation
{
    std::string Id;
    CommonFileInformation File;
};

struct CommonProductOrganization
{
    std::vector<CommonFileInformation> ImageFiles;
    std::vector<CommonFileInformation> QuickLookFiles;
    std::vector<CommonAnnexInformation> AnnexFiles;
};

struct CommonSize
{
    std::string Lines;
    std::string Columns;
    std::string Bands;
};

struct CommonBand
{
    std::string Id;
    std::string Name;
};

struct CommonGeoPosition
{
    std::string UnitLengthX;
    std::string UnitLengthY;
    std::string DimensionX;
    std::string DimensionY;
};


struct CommonProductSampling
{
    std::string ByLineUnit;
    std::string ByLineValue;
    std::string ByColumnUnit;
    std::string ByColumnValue;
};

struct CommonResolution
{
    std::string Id;
    CommonSize Size;
    CommonGeoPosition GeoPosition;
    CommonProductSampling ProductSampling;
    std::vector<CommonBand> Bands;
};

struct CommonAnglePair
{
    std::string ZenithUnit;
    std::string AzimuthUnit;
    double ZenithValue;
    double AzimuthValue;
};

struct CommonMeanViewingIncidenceAngle
{
    std::string BandId;
    CommonAnglePair Angles;
};

struct CommonAngleList
{
    std::string ColumnUnit;
    std::string ColumnStep;
    std::string RowUnit;
    std::string RowStep;
    std::vector<std::vector<double> > Values;
};

struct CommonAngles
{
    CommonAngleList Zenith;
    CommonAngleList Azimuth;
};

struct CommonViewingAnglesGrid
{
    std::string BandId;
    std::string DetectorId;
    CommonAngles Angles;
};

struct CommonBandResolution
{
    std::string BandName;
    std::string Unit;
    std::string Resolution;
};

struct CommonBandWavelength
{
    std::string BandName;
    std::string Unit;
    std::string WaveLength;
    std::string MaxUnit;
    std::string MaxWaveLength;
    std::string MinUnit;
    std::string MinWaveLength;
};

