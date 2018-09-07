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
 
#include <cmath>

#define BOOST_TEST_MODULE TileMetadataWriter
#include <boost/test/unit_test.hpp>

#include "TileMetadataWriter.hpp"

BOOST_AUTO_TEST_CASE(TileMetadataWriter)
{
    auto writer = itk::TileMetadataWriter::New();

    TileFileMetadata metadata;

    metadata.TileID = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDP_N01.00";
    metadata.TileGeometricInfo.HorizontalCSName = "WGS84 / UTM zone 31N";
    metadata.TileGeometricInfo.HorizontalCSCode = "EPSG:32631";
    metadata.ProductLevel = "L1C";

    metadata.TileGeometricInfo.TileSizeList.reserve(2);
    TileSize tileSizeEl;
    tileSizeEl.resolution = 10;
    tileSizeEl.nrows = 10980;
    tileSizeEl.ncols = 10980;

    metadata.TileGeometricInfo.TileSizeList.emplace_back(tileSizeEl);

    tileSizeEl.resolution = 20;
    tileSizeEl.nrows = 5490;
    tileSizeEl.ncols = 5490;

    metadata.TileGeometricInfo.TileSizeList.emplace_back(tileSizeEl);

    metadata.TileGeometricInfo.TileGeopositionList.reserve(2);

    TileGeoposition tileGeoposition;
    tileGeoposition.resolution = 10;
    tileGeoposition.ulx = 399960;
    tileGeoposition.uly = 3400020;
    tileGeoposition.xdim = 10;
    tileGeoposition.ydim = -10;
    metadata.TileGeometricInfo.TileGeopositionList.emplace_back(tileGeoposition);

    tileGeoposition.resolution = 20;
    tileGeoposition.ulx = 399960;
    tileGeoposition.uly = 3400020;
    tileGeoposition.xdim = 20;
    tileGeoposition.ydim = -20;
    metadata.TileGeometricInfo.TileGeopositionList.emplace_back(tileGeoposition);


    metadata.TileThematicInfo = "AKJSDGSDJFG LKSDSKDIHFKJ FKHDKUJFG";
    metadata.TileImageContentQI.NoDataPixelPercentange = "0";
    metadata.TileImageContentQI.SaturatedDefectivePixelPercentange = "1";
    metadata.TileImageContentQI.CloudShadowPercentange = "2";
    metadata.TileImageContentQI.VegetationPercentange = "3";
    metadata.TileImageContentQI.WaterPercentange = "4";
    metadata.TileImageContentQI.LowProbaCloudsPercentange = "5";
    metadata.TileImageContentQI.MediumProbaCloudsPercentange = "6";
    metadata.TileImageContentQI.HighProbaCloudsPercentange = "7";
    metadata.TileImageContentQI.ThinCirrusPercentange = "8";
    metadata.TileImageContentQI.SnowIcePercentange = "9";

    metadata.TileMasksList.reserve(2);

    TileMask tileMask;
    tileMask.MaskType = "MSK_DEFECT";
    tileMask.BandId = 0;
    tileMask.Geometry = "FULL_RESOLUTION";
    tileMask.MaskFileName = "S2A_OPER_MSK_DEFECT_MTI__20150627T180307_A000062_T31RDP_B01_MSIL1C.gml";
    metadata.TileMasksList.emplace_back(tileMask);

    tileMask.MaskType = "MSK_NODATA";
    tileMask.BandId = 22;
    tileMask.Geometry = "FULL_RESOLUTION";
    tileMask.MaskFileName = "S2A_OPER_MSK_NODATA_MTI__20150627T180307_A000062_T31RDP_B01_MSIL1C.gml";
    metadata.TileMasksList.emplace_back(tileMask);


    writer->WriteTileMetadata(metadata, "test_tileWriter.xml");

}
