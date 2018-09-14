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
    std::string NoDataPixelPercentange;
    std::string SaturatedDefectivePixelPercentange;
    std::string CloudShadowPercentange;
    std::string VegetationPercentange;
    std::string WaterPercentange;
    std::string LowProbaCloudsPercentange;
    std::string MediumProbaCloudsPercentange;
    std::string HighProbaCloudsPercentange;
    std::string ThinCirrusPercentange;
    std::string SnowIcePercentange;
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
    std::string ProductLevel;
};
