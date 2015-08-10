#include <sstream>
#include <utility>
#include <vector>

#include "otbMacro.h"

#include "SPOT4MetadataReader.hpp"
#include "tinyxml_utils.hpp"

static SPOT4Header ReadHeader(const TiXmlElement *el);
static SPOT4Files ReadFiles(const TiXmlElement *el);
static SPOT4Radiometry ReadRadiometry(const TiXmlElement *el);

namespace itk
{
std::unique_ptr<SPOT4Metadata> SPOT4MetadataReader::ReadMetadata(const std::string &path)
{
    TiXmlDocument doc(path);
    if (!doc.LoadFile()) {
        itkExceptionMacro(<< "Can't open metadata file");
    }

    return ReadMetadataXml(doc);
}

std::unique_ptr<SPOT4Metadata> SPOT4MetadataReader::ReadMetadataXml(const TiXmlDocument &doc)
{
    TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));

    auto root = hDoc.FirstChildElement("METADATA").ToElement();

    if (!root) {
        return nullptr;
    }

    auto metadata = std::unique_ptr<SPOT4Metadata>(new SPOT4Metadata);
    metadata->Header = ReadHeader(root->FirstChildElement("HEADER"));
    metadata->Files = ReadFiles(root->FirstChildElement("FILES"));
    metadata->Radiometry = ReadRadiometry(root->FirstChildElement("RADIOMETRY"));

    return metadata;
}
}

static SPOT4Header ReadHeader(const TiXmlElement *el)
{
    SPOT4Header result;

    if (!el) {
        return result;
    }

    result.DatePdv = GetChildText(el, "DATE_PDV");
    result.DateProd = GetChildText(el, "DATE_PROD");

    return result;
}

static SPOT4Files ReadFiles(const TiXmlElement *el)
{
    SPOT4Files result;

    if (!el) {
        return result;
    }

    result.GeoTIFF = GetChildText(el, "GEOTIFF");
    result.OrthoSurfAOT = GetChildText(el, "ORTHO_SURF_AOT");
    result.OrthoSurfCorrEnv = GetChildText(el, "ORTHO_SURF_CORR_ENV");
    result.OrthoSurfCorrPente = GetChildText(el, "ORTHO_SURF_CORR_PENTE");
    result.OrthoVapEau = GetChildText(el, "ORTHO_VAP_EAU");
    result.MaskSaturation = GetChildText(el, "MASK_SATURATION");
    result.MaskGapSlc = GetChildText(el, "MASK_GAP_SLC");
    result.MaskN2 = GetChildText(el, "MASK_N2");
    result.Prive = GetChildText(el, "PRIVE");

    return result;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> result;

    std::istringstream is(s);
    std::string item;
    while (std::getline(is, item, delim)) {
        result.emplace_back(std::move(item));
    }

    return result;
}

static SPOT4Radiometry ReadRadiometry(const TiXmlElement *el)
{
    SPOT4Radiometry result;

    if (!el) {
        return result;
    }

    result.Bands = split(GetChildText(el, "BANDS"), ';');

    return result;
}
