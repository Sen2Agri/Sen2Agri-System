#pragma once

#include <string>
#include <vector>

struct SPOT4Header
{
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

struct SPOT4Radiometry
{
    std::vector<std::string> Bands;
};

struct SPOT4Metadata
{
    SPOT4Header Header;
    SPOT4Files Files;
    SPOT4Radiometry Radiometry;
};
