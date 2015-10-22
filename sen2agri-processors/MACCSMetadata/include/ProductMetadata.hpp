#pragma once

#include <string>
#include <vector>

struct Bbox
{
    float LowerCornerLat;
    float LowerCornerLon;
    float UpperCornerLat;
    float UpperCornerLon;
};

struct Band
{
    int Resolution;
    std::string BandName;
};

struct AuxList
{
    std::string ProductLevel;
    std::string GIPP;
};

struct QueryOptionsContent
{
    Bbox AreaOfInterest;
    bool PreviewImage;
    std::vector<Band> BandList;
    std::string MetadataLevel;
    AuxList AuxListContent;
    std::string ProductFormat;
    bool AggregationFlag;
};

struct Granule
{
    std::string GranuleIdentifier;
    std::string ImageFormat;
    std::vector<std::string> ImageIDList;
};

struct ProductInfoMetadata
{
    std::string ProductURI;
    std::string ProcessingLevel;
    std::string ProductType;
    std::string ProcessingBaseline;
    std::string GenerationTime;
    std::string PreviewImageURL;
    std::string SpacecraftName;
    QueryOptionsContent QueryOptions;
    std::vector<Granule> ProductOrganisation;
};

struct SpecialValues
{
    std::string SpecialValueText;
    int SpecialValueIndex;
};

struct ProductImageDisplayOrder
{
    int RedChannel;
    int GreenChannel;
    int BlueChannel;
};

struct ProductImageCharacteristicsMetadata
{
    std::vector<SpecialValues> SpecialValuesList;
    ProductImageDisplayOrder ImageDisplayOrder;
    int QuantificationValue;
    std::string QuantificationUnit;
};

struct GeneralInfoMetadata
{
    ProductInfoMetadata ProductInfo;
    ProductImageCharacteristicsMetadata ProductImageCharacteristics;
};

struct GIPPInfo
{
    std::string GIPPFileName;
    std::string GIPPType;
    std::string GIPPVersion;
};

struct AuxiliaryDataInfoMetadata
{
    std::vector<GIPPInfo> GIPPList;
};

struct TechnicalQualityAssessmentMetadata
{
    int DegratedANCDataPercentage;
    int DegratedMSIDataPercentage;
};

struct QualityInspectionsMetadata
{
    std::string SensorQualityFlag;
    std::string GeometricQualityFlag;
    std::string GeneralQualityFlag;
    std::string FormatCorectnessFlag;
    std::string RadiometricQualityFlag;
};

struct GranuleReport
{
    std::string GranuleReportId;
    std::string GranuleReportFileName;
};

struct QualityControlChecksMetadata
{
    QualityInspectionsMetadata QualityInspections;
    std::vector<GranuleReport> FailedInspections;
};

struct QualityIndicatorsInfoMetadata
{
    float CloudCoverage;
    TechnicalQualityAssessmentMetadata TechnicalQualityAssessment;
    QualityControlChecksMetadata QualityControlChecks;
};

struct ProductFootprintMetadata
{
    std::vector<double> ExtPosList;
    std::string RatserCSType;
    int PixelOrigin;
};

struct CoordReferenceSystemMetadata
{
    std::string HorizCSName;
    std::string HorizCSCode;
};

struct GeometricInfoMetadata
{
    ProductFootprintMetadata ProductFootprint;
    CoordReferenceSystemMetadata CoordReferenceSystem;
};

struct ProductFileMetadata
{
    GeneralInfoMetadata GeneralInfo;
    GeometricInfoMetadata GeometricInfo;
    AuxiliaryDataInfoMetadata AuxiliaryDataInfo;
    QualityIndicatorsInfoMetadata QualityIndicatorsInfo;
};
