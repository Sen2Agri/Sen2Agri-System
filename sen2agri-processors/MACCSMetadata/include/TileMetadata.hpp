#pragma once

#include <string>
#include <vector>

struct TileSize
{
       int resolution;
       int nrows;
       int ncols;
};

struct TileGeoposition
{
       int resolution;
       int ulx;
       int uly;
       float xdim;
       float ydim;
};

struct TileGeocoding
{
    std::string HorizontalCSName;
    std::string HorizontalCSCode;
    std::vector<TileSize> TileSizeList;
    std::vector<TileGeoposition> TileGeopositionList;
};

struct TileImageContent
{
    float NoDataPixelPercentange;
    float SaturatedDefectivePixelPercentange;
    float CloudShadowPercentange;
    float VegetationPercentange;
    float WaterPercentange;
    float LowProbaCloudsPercentange;
    float MediumProbaCloudsPercentange;
    float HighProbaCloudsPercentange;
    float ThinCirrusPercentange;
    float SnowIcePercentange;
};

struct TileMask
{
    std::string MaskType;
    int BandId;
    std::string Geometry;
    std::string MaskFileName;
};

struct TileFileMetadata
{
    std::string TileID;
    TileGeocoding TileGeometricInfo;
    std::string TileThematicInfo;
    TileImageContent TileImageContentQI;
    std::vector<TileMask>TileMasksList;
};
