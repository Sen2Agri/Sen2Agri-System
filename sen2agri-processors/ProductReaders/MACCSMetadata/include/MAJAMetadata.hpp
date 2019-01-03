#pragma once

#include <string>
#include <vector>
#include "CommonMetadata.hpp"
#include "MACCSMetadata.hpp"

struct MAJAMetadataIdentification
{
    std::string MetadataFormat;
    std::string MetdataFormatVersion;
    std::string MetadataProfile;
    std::string MetadataInformation;
};

struct MAJADatasetIdentification
{
    std::string Identifier;
    std::string Authority;
    std::string Producer;
    std::string Project;
    std::string GeographicalZoneTile;
};

struct MAJAProductCharacteristics
{
    std::string AcquisitionDateTime;
    std::string Mission;
    std::vector<CommonResolution> GroupResolutions;
};

struct MAJAGeopositionInformation
{
    CommonSize Size;
    CommonGeoPosition GeoPosition;
    CommonProductSampling ProductSampling;
};

struct MAJAGeometricInformation
{
    CommonAnglePair MeanSunAngle;
    CommonAngles SolarAngles;
    std::vector<CommonMeanViewingIncidenceAngle> MeanViewingIncidenceAngles;
    std::vector<CommonViewingAnglesGrid> ViewingAngles;
};

struct MAJARadiometricInformation
{
    std::string ReflectanceQuantificationValue;
    std::string NoDataValue;
    std::string VAPNoDataValue;
    std::string VAPQuantificationValue;
    std::string AOTNoDataValue;
    std::string AOTQuantificationValue;
    std::vector<CommonBandResolution> BandResolutions;
    std::vector<CommonBandWavelength> BandWavelengths;
};

struct MAJAFileMetadata
{
    MAJAMetadataIdentification MetadataIdentification;
    MAJADatasetIdentification DatasetIdentification;
    MAJAProductCharacteristics ProductCharacteristics;
    CommonProductOrganization ProductOrganization;
    MAJAGeometricInformation GeometricInformation;
    MAJARadiometricInformation RadiometricInformation;
    std::string ProductPath;
};
