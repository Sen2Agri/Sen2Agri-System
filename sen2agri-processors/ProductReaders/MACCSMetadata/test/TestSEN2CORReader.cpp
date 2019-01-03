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

#define BOOST_TEST_MODULE SEN2CORReader
#include <boost/test/unit_test.hpp>

#include "SEN2CORMetadataReader.hpp"
#include "ViewingAngles.hpp"

BOOST_AUTO_TEST_CASE(SEN2CORReader)
{
    auto reader = itk::SEN2CORMetadataReader::New();

    auto m = reader->ReadMetadata("MTD_MSIL2A.xml");

    BOOST_CHECK_EQUAL(m->InstanceId.AcquisitionDate, "20181127");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.Mission, "Sentinel-2B");

    //image files
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R10m/T28RBN_20181127T115219_B02_10m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].LogicalName, "T28RBN_20181127T115219_B02_10m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R10m/T28RBN_20181127T115219_B03_10m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].LogicalName, "T28RBN_20181127T115219_B03_10m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R10m/T28RBN_20181127T115219_B04_10m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].LogicalName, "T28RBN_20181127T115219_B04_10m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R10m/T28RBN_20181127T115219_B08_10m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].LogicalName, "T28RBN_20181127T115219_B08_10m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[4].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[4].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R10m/T28RBN_20181127T115219_TCI_10m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[4].LogicalName, "T28RBN_20181127T115219_TCI_10m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[15].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[15].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R20m/T28RBN_20181127T115219_B12_20m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[15].LogicalName, "T28RBN_20181127T115219_B12_20m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[33].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[33].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R60m/T28RBN_20181127T115219_WVP_60m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[33].LogicalName, "T28RBN_20181127T115219_WVP_60m");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[34].Nature, "PIC");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[34].FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/IMG_DATA/R60m/T28RBN_20181127T115219_SCL_60m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[34].LogicalName, "T28RBN_20181127T115219_SCL_60m");

    BOOST_CHECK_EQUAL(m->ProductInformation.AcquisitionDateTime, "2018-11-27T11:52:19.024Z");

    BOOST_ASSERT(m->ProductInformation.BandResolutions.size() == 13);
    BOOST_ASSERT(m->ProductInformation.BandWavelengths.size() == 13);
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].BandName, "B1");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].Resolution, "60");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].BandName, "B1");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MinWaveLength, "411");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MaxWaveLength, "456");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].WaveLength, "442.3");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].BandName, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].BandName, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MinWaveLength, "438");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MaxWaveLength, "532");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].WaveLength, "492.1");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].BandName, "B3");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].BandName, "B3");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MinWaveLength, "536");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MaxWaveLength, "582");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].WaveLength, "559");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].BandName, "B4");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].BandName, "B4");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MinWaveLength, "646");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MaxWaveLength, "685");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].WaveLength, "665");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].BandName, "B5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].BandName, "B5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MinWaveLength, "694");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MaxWaveLength, "714");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].WaveLength, "703.8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].BandName, "B6");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].BandName, "B6");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MinWaveLength, "730");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MaxWaveLength, "748");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].WaveLength, "739.1");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].BandName, "B7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].BandName, "B7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MinWaveLength, "766");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MaxWaveLength, "794");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].WaveLength, "779.7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].BandName, "B8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].BandName, "B8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MinWaveLength, "774");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MaxWaveLength, "907");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].WaveLength, "833");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].BandName, "B8A");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].BandName, "B8A");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MinWaveLength, "848");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MaxWaveLength, "880");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].WaveLength, "864");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].BandName, "B9");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].Resolution, "60");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].BandName, "B9");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MinWaveLength, "930");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MaxWaveLength, "957");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].WaveLength, "943.2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[10].BandName, "B10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[10].Resolution, "60");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[10].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].BandName, "B10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].MinWaveLength, "1339");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].MaxWaveLength, "1415");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[10].WaveLength, "1376.9");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[11].BandName, "B11");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[11].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[11].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].BandName, "B11");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].MinWaveLength, "1538");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].MaxWaveLength, "1679");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[11].WaveLength, "1610.4");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[12].BandName, "B12");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[12].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[12].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].BandName, "B12");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].MinWaveLength, "2065");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].MaxWaveLength, "2303");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[12].WaveLength, "2185.7");

    BOOST_CHECK_EQUAL(m->ProductInformation.ReflectanceQuantificationValue, "10000");

    BOOST_CHECK_EQUAL(m->ImageInformation.AOTQuantificationValue, "1000.0");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPQuantificationValue, "1000.0");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTNoDataValue, "0");

    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Id, "0");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Name, "B1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Name, "B2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[2].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[2].Name, "B3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[3].Id, "3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[3].Name, "B4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[4].Id, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[4].Name, "B5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[5].Id, "5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[5].Name, "B6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[6].Id, "6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[6].Name, "B7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[7].Id, "7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[7].Name, "B8");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[8].Id, "8");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[8].Name, "B8A");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[9].Id, "9");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[9].Name, "B9");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[10].Id, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[10].Name, "B10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[11].Id, "11");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[11].Name, "B11");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[12].Id, "12");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[12].Name, "B12");

    m = reader->ReadMetadata("MTD_TL.xml");
    BOOST_REQUIRE(m);
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Id, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Lines, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Columns, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthX, "199980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthY, "2800020");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionX, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionY, "-10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineValue, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnValue, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Id, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Lines, "5490");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Columns, "5490");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthX, "199980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthY, "2800020");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionX, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionY, "-20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineValue, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnValue, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].Id, "60");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].Size.Lines, "1830");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].Size.Columns, "1830");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].GeoPosition.UnitLengthX, "199980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].GeoPosition.UnitLengthY, "2800020");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].GeoPosition.DimensionX, "60");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].GeoPosition.DimensionY, "-60");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].ProductSampling.ByLineUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].ProductSampling.ByLineValue, "60");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].ProductSampling.ByColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[2].ProductSampling.ByColumnValue, "60");

    BOOST_ASSERT(m->ProductOrganization.AnnexFiles.size() == 70);
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].Id, "0");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "MSK_DEFECT");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/QI_DATA/MSK_DEFECT_B01.gml");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.LogicalName, "MSK_DEFECT_B01");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].Id, "0");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.Nature, "MSK_SATURA");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/QI_DATA/MSK_SATURA_B01.gml");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.LogicalName, "MSK_SATURA_B01");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[68].File.Nature, "MSK_CLDPRB");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[68].File.FileLocation, "./GRANULE/L2A_T28RBN_A009015_20181127T115214/QI_DATA/MSK_CLDPRB_60m.jp2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[68].File.LogicalName, "MSK_CLDPRB_60m");

    //angles
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithValue, 48.4167095795612);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthValue, 160.338531925883);

    BOOST_ASSERT(m->ProductInformation.MeanViewingIncidenceAngles.size() == 13);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithValue, 3.43920287925967);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthValue, 136.479098083673);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].BandId, "9");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.ZenithValue, 3.53081603529832);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.AzimuthValue, 136.199264135978);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].BandId, "7");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.ZenithValue, 2.87104189924021);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.AzimuthValue, 141.879280693854);


    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles.size(), 78);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].DetectorId, "3");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][0],
                      7.4926); // HACK fp precision
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][5]));

    const auto &viewingAngles = ComputeViewingAngles(m->ProductInformation.ViewingAngles);
    BOOST_CHECK_EQUAL(viewingAngles.size(), 13);
    BOOST_CHECK_EQUAL(viewingAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(viewingAngles[12].BandId, "12");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.RowStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.RowStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.Values[0][0], 7.4926);
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.Values[0][0], 117.18);

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowStep, "5000");

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0][0], 49.053);

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowStep, "5000");

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0][0],
                      159.802); // HACK fp precision
}
