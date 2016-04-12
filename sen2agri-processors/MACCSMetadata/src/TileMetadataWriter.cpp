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
 
#include "TileMetadataWriter.hpp"

namespace itk
{

XElement Format(const TileGeocoding &tileGeocoding)
{
    XElement tileGeocodingEl("Tile_Geocoding", XAttribute("metadataLevel", "Brief"),
                                    XElement("HORIZONTAL_CS_NAME", tileGeocoding.HorizontalCSName),
                                    XElement("HORIZONTAL_CS_CODE", tileGeocoding.HorizontalCSCode));

    for (const auto &tileSizes : tileGeocoding.TileSizeList) {
        tileGeocodingEl.Append(
                    XElement(
                        "Size", XAttribute("resolution", std::to_string(tileSizes.resolution)),
                                XElement("NROWS", std::to_string(tileSizes.nrows)),
                                XElement("NCOLS", std::to_string(tileSizes.ncols))));
    }

    for (const auto &tileGeopositions : tileGeocoding.TileGeopositionList) {
        tileGeocodingEl.Append(
                    XElement(
                        "Geoposition", XAttribute("resolution", std::to_string(tileGeopositions.resolution)),
                                XElement("ULX", std::to_string(tileGeopositions.ulx)),
                                XElement("ULY", std::to_string(tileGeopositions.uly)),
                                XElement("XDIM", std::to_string(tileGeopositions.xdim)),
                                XElement("YDIM", std::to_string(tileGeopositions.ydim))));
    }
    return tileGeocodingEl;
}

XElement Format(const TileImageContent &tileImageContent)
{
    XElement tileImageContentEl("Image_Content_QI",
                   XElement("NODATA_PIXEL_PERCENTAGE", tileImageContent.NoDataPixelPercentange),
                   XElement("SATURATED_DEFECTIVE_PIXEL_PERCENTAGE", tileImageContent.SaturatedDefectivePixelPercentange),
                   XElement("CLOUD_SHADOW_PERCENTAGE", tileImageContent.CloudShadowPercentange),
                   XElement("VEGETATION_PERCENTAGE", tileImageContent.VegetationPercentange),
                   XElement("WATER_PERCENTAGE", tileImageContent.WaterPercentange),
                   XElement("LOW_PROBA_CLOUDS_PERCENTAGE", tileImageContent.LowProbaCloudsPercentange),
                   XElement("MEDIUM_PROBA_CLOUDS_PERCENTAGE", tileImageContent.MediumProbaCloudsPercentange),
                   XElement("HIGH_PROBA_CLOUDS_PERCENTAGE", tileImageContent.HighProbaCloudsPercentange),
                   XElement("THIN_CIRRUS_PERCENTAGE", tileImageContent.ThinCirrusPercentange),
                   XElement("SNOW_ICE_PERCENTAGE", tileImageContent.SnowIcePercentange));

    return tileImageContentEl;
}

XElement Format(const std::vector<TileMask> &tileMaskList)
{
    XElement tileMasksEl("Pixel_Level_QI");

    for (const auto &tileMask : tileMaskList) {
        tileMasksEl.Append(XElement("MASK_FILENAME", XAttribute("type", tileMask.MaskType),
                                                     XAttribute("bandId", std::to_string(tileMask.BandId)),
                                                     XAttribute("geometry", tileMask.Geometry), XText(tileMask.MaskFileName)));
    }

    return tileMasksEl;
}


XDocument TileMetadataWriter::CreateTileMetadataXml(const TileFileMetadata &metadata)
{

    XDocument doc(XDeclaration("1.0", "UTF-8", ""),
                  XUnknown("<?xml-stylesheet type=\"text/xsl\" href=\"DISPLAY/display.xsl\"?>"));

    XElement root(metadata.ProductLevel + "_Tile_ID",
                      XAttribute("xmlns", "http://eop-cfi.esa.int/CFI"),
                      XAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance"));

    root.Append(
        XElement(
           "General_Info", XElement("TILE_ID", XAttribute("metadataLevel", "Brief"), XText(metadata.TileID))));
    root.Append(
            XElement(
            "Geometric_Info", Format(metadata.TileGeometricInfo)));

    root.Append(
            XElement(
             "Thematic_Info", XText(metadata.TileThematicInfo)));

    root.Append(
            XElement(
              "Quality_Indicators_Info", XAttribute("metadataLevel", "Standard"), Format(metadata.TileImageContentQI),
                    Format(metadata.TileMasksList)));

    doc.Append(root);


    return doc;
}

void TileMetadataWriter::WriteTileMetadata(const TileFileMetadata &metadata, const std::string &path)
{
    CreateTileMetadataXml(metadata).Save(path);
}
}
