/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#include <sstream>
#include <utility>
#include <vector>
#include <limits>

#include "otbMacro.h"

#include "SPOT4MetadataReader.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

static SPOT4Header ReadHeader(const TiXmlElement *el);
static SPOT4Files ReadFiles(const TiXmlElement *el);
static SPOT4Geometry ReadGeometry(const TiXmlElement *el);
static SPOT4WGS84 ReadWGS84(const TiXmlElement *el);
static SPOT4Radiometry ReadRadiometry(const TiXmlElement *el);

namespace itk
{
std::unique_ptr<SPOT4Metadata> SPOT4MetadataReader::ReadMetadata(const std::string &path)
{
    TiXmlDocument doc(path);
    if (!doc.LoadFile()) {
        return nullptr;
    }

    auto metadata = ReadMetadataXml(doc);
    if (metadata) {
        metadata->ProductPath = path;
    }
    return metadata;
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
    metadata->Geometry = ReadGeometry(root->FirstChildElement("GEOMETRY"));
    metadata->WGS84 = ReadWGS84(root->FirstChildElement("WGS84"));
    metadata->Radiometry = ReadRadiometry(root->FirstChildElement("RADIOMETRY"));

    // SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_DIV.TIF

    if (metadata->Files.OrthoSurfAOT.empty()) {
        auto s = metadata->Header.Ident;
        auto pos = s.find("N2A_");
        if (pos != std::string::npos) {
            s.replace(pos, 4, "N2A_AOT_");
        }

        metadata->Files.OrthoSurfAOT = std::move(s) + ".TIF";
    }

    if (metadata->Files.MaskDiv.empty()) {
        metadata->Files.MaskDiv =
            metadata->Files.MaskN2 + '/' + metadata->Header.Ident + "_DIV.TIF";
    }

    if (metadata->Files.MaskNua.empty()) {
        metadata->Files.MaskNua =
            metadata->Files.MaskN2 + '/' + metadata->Header.Ident + "_NUA.TIF";
    }

    if (metadata->Files.MaskSaturation.empty()) {
        metadata->Files.MaskSaturation =
            metadata->Files.MaskN2 + '/' + metadata->Header.Ident + "_SAT.TIF";
    }

    return metadata;
}
}

static SPOT4Header ReadHeader(const TiXmlElement *el)
{
    SPOT4Header result;

    if (!el) {
        return result;
    }

    result.Ident = GetChildText(el, "IDENT");
    result.DatePdv = GetChildText(el, "DATE_PDV");
    result.DateProd = GetChildText(el, "DATE_PROD");
    result.Mission = GetChildText(el, "PLATEFORM");
    result.Instrument = GetChildText(el, "SENSOR");

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

static SPOT4Geometry ReadGeometry(const TiXmlElement *el)
{
    SPOT4Geometry result;

    if (!el) {
        return result;
    }

    result.Resolution = GetChildText(el, "RESOLUTION");
    result.NbCols = GetChildText(el, "NB_COLS");
    result.NbRows = GetChildText(el, "NB_ROWS");

    return result;
}

static SPOT4WGS84 ReadWGS84(const TiXmlElement *el)
{
    SPOT4WGS84 result;
    result.HGX = std::numeric_limits<double>::quiet_NaN();
    result.HGY = std::numeric_limits<double>::quiet_NaN();
    result.HDX = std::numeric_limits<double>::quiet_NaN();
    result.HDY = std::numeric_limits<double>::quiet_NaN();
    result.BGX = std::numeric_limits<double>::quiet_NaN();
    result.BGY = std::numeric_limits<double>::quiet_NaN();
    result.BDX = std::numeric_limits<double>::quiet_NaN();
    result.BDY = std::numeric_limits<double>::quiet_NaN();

    if (!el) {
        return result;
    }

    result.HGX = ReadDouble(GetChildText(el, "HGX"));
    result.HGY = ReadDouble(GetChildText(el, "HGY"));
    result.HDX = ReadDouble(GetChildText(el, "HDX"));
    result.HDY = ReadDouble(GetChildText(el, "HDY"));
    result.BGX = ReadDouble(GetChildText(el, "BGX"));
    result.BGY = ReadDouble(GetChildText(el, "BGY"));
    result.BDX = ReadDouble(GetChildText(el, "BDX"));
    result.BDY = ReadDouble(GetChildText(el, "BDY"));

    return result;
}

static SPOT4Angles ReadAngles(const TiXmlElement *el)
{
    SPOT4Angles result;
    result.PhiS = std::numeric_limits<double>::quiet_NaN();
    result.ThetaS = std::numeric_limits<double>::quiet_NaN();
    result.PhiV = std::numeric_limits<double>::quiet_NaN();
    result.ThetaV = std::numeric_limits<double>::quiet_NaN();
    result.Pitch = std::numeric_limits<double>::quiet_NaN();
    result.Roll = std::numeric_limits<double>::quiet_NaN();

    if (!el) {
        return result;
    }

    result.PhiS = ReadDouble(GetChildText(el, "PHI_S"));
    result.ThetaS = ReadDouble(GetChildText(el, "THETA_S"));
    result.PhiV = ReadDouble(GetChildText(el, "PHI_V"));
    result.ThetaV = ReadDouble(GetChildText(el, "THETA_V"));
    result.Pitch = ReadDouble(GetChildText(el, "PITCH"));
    result.Roll = ReadDouble(GetChildText(el, "ROLL"));

    return result;
}

static SPOT4Radiometry ReadRadiometry(const TiXmlElement *el)
{
    SPOT4Radiometry result;

    if (!el) {
        return result;
    }

    result.Bands = split(GetChildText(el, "BANDS"), ';');
    result.Angles = ReadAngles(el->FirstChildElement("ANGLES"));

    return result;
}
