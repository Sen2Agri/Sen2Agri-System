#pragma once

#include <string>
#include <vector>

struct SPOT4Header
{
    std::string Ident;
    std::string DatePdv;
    std::string DateProd;
};

struct SPOT4Files
{
    std::string GeoTIFF;
    std::string OrthoSurfAOT;
    std::string OrthoSurfCorrEnv;
    std::string OrthoSurfCorrPente;
    std::string OrthoVapEau;
    std::string MaskSaturation;
    std::string MaskGapSlc;
    std::string MaskN2;
    std::string Prive;
};

struct SPOT4Geometry
{
    std::string Resolution;
    std::string NbCols;
    std::string NbRows;
};

struct SPOT4Radiometry
{
    std::vector<std::string> Bands;
};

struct SPOT4Metadata
{
    SPOT4Header Header;
    SPOT4Files Files;
    SPOT4Geometry Geometry;
    SPOT4Radiometry Radiometry;
};
