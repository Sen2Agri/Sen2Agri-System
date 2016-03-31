#pragma once

#include <string>
#include <vector>

struct SPOT4Header
{
    std::string Ident;
    std::string DatePdv;
    std::string DateProd;
    std::string Mission;
    std::string Instrument;
};

struct SPOT4Files
{
    std::string GeoTIFF;
    std::string OrthoSurfAOT;
    std::string OrthoSurfCorrEnv;
    std::string OrthoSurfCorrPente;
    std::string OrthoVapEau;
    std::string MaskDiv;
    std::string MaskNua;
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

struct SPOT4WGS84
{
    double HGX;
    double HGY;
    double HDX;
    double HDY;
    double BGX;
    double BGY;
    double BDX;
    double BDY;
};

struct SPOT4Angles
{
    double PhiS;
    double ThetaS;
    double PhiV;
    double ThetaV;
    double Pitch;
    double Roll;
};

struct SPOT4Radiometry
{
    std::vector<std::string> Bands;
    SPOT4Angles Angles;
};

struct SPOT4Metadata
{
    SPOT4Header Header;
    SPOT4Files Files;
    SPOT4Geometry Geometry;
    SPOT4WGS84 WGS84;
    SPOT4Radiometry Radiometry;

    std::string ProductPath;
};
