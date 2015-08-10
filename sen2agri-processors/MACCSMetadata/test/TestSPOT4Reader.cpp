#include <cassert>

#include "SPOT4MetadataReader.hpp"

int main()
{
    auto reader = itk::SPOT4MetadataReader::New();

    auto m = reader->ReadMetadata("SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml");

    assert(m);

    assert(m->Header.DatePdv == "2013-03-18 09:54:26");
    assert(m->Header.DateProd == "2014-03-05 03:45:20.328949");

    assert(m->Files.GeoTIFF == "");
    assert(m->Files.OrthoSurfAOT == "");
    assert(m->Files.OrthoSurfCorrEnv == "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_ENV_EBelgiumD0000B0000.TIF");
    assert(m->Files.OrthoSurfCorrPente == "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_PENTE_EBelgiumD0000B0000.TIF");
    assert(m->Files.OrthoVapEau == "");
    assert(m->Files.MaskSaturation == "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_SAT.TIF");
    assert(m->Files.MaskGapSlc == "");
    assert(m->Files.MaskN2 == "MASK");
    assert(m->Files.Prive == "PRIVE");

    assert(m->Radiometry.Bands.size() == 4);
    assert(m->Radiometry.Bands[0] == "XS1");
    assert(m->Radiometry.Bands[1] == "XS2");
    assert(m->Radiometry.Bands[2] == "XS3");
    assert(m->Radiometry.Bands[3] == "SWIR");
}
