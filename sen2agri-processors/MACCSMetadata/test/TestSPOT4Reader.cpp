#include <cmath>

#include <itkTestingMacros.h>

#include "SPOT4MetadataReader.hpp"

int main()
{
    auto reader = itk::SPOT4MetadataReader::New();

    auto m = reader->ReadMetadata("SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->Header.Ident, "SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000");
    TEST_EXPECT_EQUAL(m->Header.DatePdv, "2013-03-18 09:54:26");
    TEST_EXPECT_EQUAL(m->Header.DateProd, "2014-03-05 03:45:20.328949");

    TEST_EXPECT_EQUAL(m->Files.GeoTIFF, "");
    TEST_EXPECT_EQUAL(m->Files.OrthoSurfAOT,
                      "SPOT4_HRVIR1_XS_20130318_N2A_AOT_EBelgiumD0000B0000.TIF");
    TEST_EXPECT_EQUAL(m->Files.OrthoSurfCorrEnv,
                      "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_ENV_EBelgiumD0000B0000.TIF");
    TEST_EXPECT_EQUAL(m->Files.OrthoSurfCorrPente,
                      "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_PENTE_EBelgiumD0000B0000.TIF");
    TEST_EXPECT_EQUAL(m->Files.OrthoVapEau, "");
    TEST_EXPECT_EQUAL(m->Files.MaskDiv,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_DIV.TIF");
    TEST_EXPECT_EQUAL(m->Files.MaskNua,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_NUA.TIF");
    TEST_EXPECT_EQUAL(m->Files.MaskSaturation,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_SAT.TIF");
    TEST_EXPECT_EQUAL(m->Files.MaskGapSlc, "");
    TEST_EXPECT_EQUAL(m->Files.MaskN2, "MASK");
    TEST_EXPECT_EQUAL(m->Files.Prive, "PRIVE");

    TEST_EXPECT_EQUAL(m->Geometry.Resolution, "20.0");
    TEST_EXPECT_EQUAL(m->Geometry.NbCols, "4500");
    TEST_EXPECT_EQUAL(m->Geometry.NbRows, "4000");

    TEST_EXPECT_EQUAL(m->Radiometry.Bands.size(), 4);
    TEST_EXPECT_EQUAL(m->Radiometry.Bands[0], "XS1");
    TEST_EXPECT_EQUAL(m->Radiometry.Bands[1], "XS2");
    TEST_EXPECT_EQUAL(m->Radiometry.Bands[2], "XS3");
    TEST_EXPECT_EQUAL(m->Radiometry.Bands[3], "SWIR");

    TEST_EXPECT_EQUAL(m->Radiometry.Angles.PhiS, 145.43902353);
    TEST_EXPECT_EQUAL(m->Radiometry.Angles.ThetaS, 57.472591328);
    TEST_EXPECT_EQUAL(m->Radiometry.Angles.PhiV, -73.809703566);
    TEST_EXPECT_EQUAL(m->Radiometry.Angles.ThetaV, 18.141025097);
    TEST_EXPECT_TRUE(std::isnan(m->Radiometry.Angles.Pitch));
    TEST_EXPECT_TRUE(std::isnan(m->Radiometry.Angles.Roll));
}
