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

#define BOOST_TEST_MODULE MAJAReader
#include <boost/test/unit_test.hpp>

#include "MAJAMetadataReader.hpp"
#include "ViewingAngles.hpp"

BOOST_AUTO_TEST_CASE(MAJAReader)
{
    auto reader = itk::MAJAMetadataReader::New();

    auto m = reader->ReadMetadata("SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_MTD_ALL.xml");

    BOOST_REQUIRE(m);
    BOOST_CHECK_EQUAL(m->InstanceId.AcquisitionDate, "20160928");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.Mission, "SENTINEL2A");

    /*
     * specific MAJA
    //metadata identification
    BOOST_CHECK_EQUAL(m->MetadataIdentification.MetadataFormat, "METADATA_MUSCATE");
    BOOST_CHECK_EQUAL(m->MetadataIdentification.MetdataFormatVersion, "1.16");
    BOOST_CHECK_EQUAL(m->MetadataIdentification.MetadataProfile, "USER");
    BOOST_CHECK_EQUAL(m->MetadataIdentification.MetadataInformation, "EXPERT");

    //dataset identification
    BOOST_CHECK_EQUAL(m->DatasetIdentification.Identifier, "SENTINEL2A_20160928-105637-665_L2A_T31UDS_C");
    BOOST_CHECK_EQUAL(m->DatasetIdentification.Authority, "THEIA");
    BOOST_CHECK_EQUAL(m->DatasetIdentification.Producer, "MUSCATE");
    BOOST_CHECK_EQUAL(m->DatasetIdentification.Project, "SENTINEL2");
    */

    //product characteristic
    BOOST_ASSERT(m->ImageInformation.Resolutions.size() == 2);
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Id, "R1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Name, "B2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Id, "0");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Name, "B3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Name, "B4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Name, "B8");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Id, "3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthX, "399960");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthY, "5700000");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionX, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionY, "-10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Lines, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Columns, "10980");

    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Id, "R2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Name, "B5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Id, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Name, "B6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Id, "5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Name, "B7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Id, "6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Name, "B8A");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Id, "7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Name, "B11");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Id, "8");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Name, "B12");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Id, "9");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthX, "399960");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthY, "5700000");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionX, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionY, "-20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Lines, "5490");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Columns, "5490");

    //product organization
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].Nature, "QCK");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_QKL_ALL.jpg");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].LogicalName, "SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_QKL_ALL");

    BOOST_ASSERT(m->ProductOrganization.ImageFiles.size() == 24);
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "Surface_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_SRE_B2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].BandId, "B2");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "Surface_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_SRE_B3.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].BandId, "B3");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].Nature, "Surface_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_SRE_B4.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].BandId, "B4");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[8].Nature, "Surface_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[8].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_SRE_B11.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[8].BandId, "B11");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[9].Nature, "Surface_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[9].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_SRE_B12.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[9].BandId, "B12");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[10].Nature, "Flat_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[10].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_FRE_B2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[10].BandId, "B2");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[11].Nature, "Flat_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[11].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_FRE_B3.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[11].BandId, "B3");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[12].Nature, "Flat_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[12].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_FRE_B4.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[12].BandId, "B4");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[18].Nature, "Flat_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[18].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_FRE_B11.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[18].BandId, "B11");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[19].Nature, "Flat_Reflectance");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[19].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_FRE_B12.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[19].BandId, "B12");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[20].Nature, "Water_Vapor_Content");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[20].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_ATB_R1.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[20].GroupId, "R1");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[20].BandNumber, 1);

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[21].Nature, "Water_Vapor_Content");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[21].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_ATB_R2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[21].GroupId, "R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[21].BandNumber, 1);

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[22].Nature, "Aerosol_Optical_Thickness");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[22].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_ATB_R1.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[22].GroupId, "R1");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[22].BandNumber, 2);

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[23].Nature, "Aerosol_Optical_Thickness");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[23].FileLocation, "./SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_ATB_R2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[23].GroupId, "R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[23].BandNumber, 2);

    BOOST_ASSERT(m->ProductOrganization.AnnexFiles.size() == 34);
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "WVC_Interpolation");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_IAB_R1.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.GroupId, "R1");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.BitNumber, 1);

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[1].File.Nature, "WVC_Interpolation");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[1].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_IAB_R2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[1].File.GroupId, "R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[1].File.BitNumber, 1);

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[2].File.Nature, "AOT_Interpolation");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[2].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_IAB_R1.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[2].File.GroupId, "R1");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[2].File.BitNumber, 2);

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.Nature, "AOT_Interpolation");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_IAB_R2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.GroupId, "R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[3].File.BitNumber, 2);

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[4].File.Nature, "Detailed_Cloud");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[4].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_CLM_R1.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[4].File.GroupId, "R1");

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[5].File.Nature, "Detailed_Cloud");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[5].File.FileLocation, "./MASKS/SENTINEL2A_20160928-105637-665_L2A_T31UDS_C_V1-0_CLM_R2.tif");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[5].File.GroupId, "R2");

    BOOST_CHECK_EQUAL(m->ProductInformation.AcquisitionDateTime, "2016-09-28T10:56:37.665Z");

    //MAJAProductInformation ProductInformation;
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithValue, 54.0684);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthValue, 166.336);

    BOOST_ASSERT(m->ProductInformation.MeanViewingIncidenceAngles.size() == 10);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].BandId, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithValue, 8.04279);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthValue, 104.923);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].BandId, "B3");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.ZenithValue, 8.07215);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[1].Angles.AzimuthValue, 104.912);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].BandId, "B12");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.ZenithValue, 8.20193);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[9].Angles.AzimuthValue, 105.112);

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0][0], 54.6666);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0][22], 54.4449);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[22][0], 53.6943);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[22][22], 53.4712);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0][0], 165.42);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0][22], 167.324);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[22][0], 165.358);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[22][22], 167.243);

    BOOST_ASSERT(m->ProductInformation.ViewingAngles.size() == 50);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].BandId, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].DetectorId, "01");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.RowStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values.size(), 23);
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][0]));
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][22]));
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[22][0], 11.7314);
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[22][22]));
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.RowStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values[0].size(), 23);
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values[0][0]));
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values[0][22]));
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values[22][0], 106.21);
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Azimuth.Values[22][22]));

    BOOST_ASSERT(m->ProductInformation.BandResolutions.size() == 10);
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].BandName, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[0].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].BandName, "B3");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[1].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].BandName, "B4");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[2].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].BandName, "B5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[3].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].BandName, "B6");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[4].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].BandName, "B7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[5].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].BandName, "B8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].Resolution, "10");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[6].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].BandName, "B8A");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[7].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].BandName, "B11");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[8].Unit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].BandName, "B12");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].Resolution, "20");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandResolutions[9].Unit, "m");

    BOOST_ASSERT(m->ProductInformation.BandWavelengths.size() == 10);
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].BandName, "B2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MinWaveLength, "440");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].MaxWaveLength, "538");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[0].WaveLength, "496.6");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].BandName, "B3");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MinWaveLength, "537");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].MaxWaveLength, "582");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[1].WaveLength, "560");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].BandName, "B4");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MinWaveLength, "646");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].MaxWaveLength, "684");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[2].WaveLength, "664.5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].BandName, "B5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MinWaveLength, "694");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].MaxWaveLength, "713");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[3].WaveLength, "703.9");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].BandName, "B6");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MinWaveLength, "731");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].MaxWaveLength, "749");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[4].WaveLength, "740.2");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].BandName, "B7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MinWaveLength, "769");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].MaxWaveLength, "797");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[5].WaveLength, "782.5");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].BandName, "B8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MinWaveLength, "760");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].MaxWaveLength, "908");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[6].WaveLength, "835.1");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].BandName, "B8A");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MinWaveLength, "848");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].MaxWaveLength, "881");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[7].WaveLength, "864.8");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].BandName, "B11");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MinWaveLength, "1539");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].MaxWaveLength, "1682");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[8].WaveLength, "1613.7");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].BandName, "B12");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MinUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MinWaveLength, "2078");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MaxUnit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].MaxWaveLength, "2320");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].Unit, "nm");
    BOOST_CHECK_EQUAL(m->ProductInformation.BandWavelengths[9].WaveLength, "2202.4");

    BOOST_CHECK_EQUAL(m->ProductInformation.ReflectanceQuantificationValue, "10000");

    BOOST_CHECK_EQUAL(m->ImageInformation.NoDataValue, "-10000");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPQuantificationValue, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPNoDataValue, "0");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTQuantificationValue, "200");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTNoDataValue, "0");

}
