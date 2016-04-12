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

#define BOOST_TEST_MODULE SPOT4Reader
#include <boost/test/unit_test.hpp>

#include "SPOT4MetadataReader.hpp"

BOOST_AUTO_TEST_CASE(SPOT4Reader)
{
    auto reader = itk::SPOT4MetadataReader::New();

    auto m = reader->ReadMetadata("SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->Header.Ident, "SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000");
    BOOST_CHECK_EQUAL(m->Header.DatePdv, "2013-03-18 09:54:26");
    BOOST_CHECK_EQUAL(m->Header.DateProd, "2014-03-05 03:45:20.328949");

    BOOST_CHECK_EQUAL(m->Files.GeoTIFF, "");
    BOOST_CHECK_EQUAL(m->Files.OrthoSurfAOT,
                      "SPOT4_HRVIR1_XS_20130318_N2A_AOT_EBelgiumD0000B0000.TIF");
    BOOST_CHECK_EQUAL(m->Files.OrthoSurfCorrEnv,
                      "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_ENV_EBelgiumD0000B0000.TIF");
    BOOST_CHECK_EQUAL(m->Files.OrthoSurfCorrPente,
                      "SPOT4_HRVIR1_XS_20130318_N2A_ORTHO_SURF_CORR_PENTE_EBelgiumD0000B0000.TIF");
    BOOST_CHECK_EQUAL(m->Files.OrthoVapEau, "");
    BOOST_CHECK_EQUAL(m->Files.MaskDiv,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_DIV.TIF");
    BOOST_CHECK_EQUAL(m->Files.MaskNua,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_NUA.TIF");
    BOOST_CHECK_EQUAL(m->Files.MaskSaturation,
                      "MASK/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000_SAT.TIF");
    BOOST_CHECK_EQUAL(m->Files.MaskGapSlc, "");
    BOOST_CHECK_EQUAL(m->Files.MaskN2, "MASK");
    BOOST_CHECK_EQUAL(m->Files.Prive, "PRIVE");

    BOOST_CHECK_EQUAL(m->Geometry.Resolution, "20.0");
    BOOST_CHECK_EQUAL(m->Geometry.NbCols, "4500");
    BOOST_CHECK_EQUAL(m->Geometry.NbRows, "4000");

    BOOST_CHECK_EQUAL(m->WGS84.HGX, 4.35370631912);
    BOOST_CHECK_EQUAL(m->WGS84.HGY, 50.9937317946);
    BOOST_CHECK_EQUAL(m->WGS84.HDX, 5.6350667637);
    BOOST_CHECK_EQUAL(m->WGS84.HDY, 50.9718471007);
    BOOST_CHECK_EQUAL(m->WGS84.BGX, 4.33320150614);
    BOOST_CHECK_EQUAL(m->WGS84.BGY, 50.2744843509);
    BOOST_CHECK_EQUAL(m->WGS84.BDX, 5.59519345898);
    BOOST_CHECK_EQUAL(m->WGS84.BDY, 50.2531490304);

    BOOST_CHECK_EQUAL(m->Radiometry.Bands.size(), 4);
    BOOST_CHECK_EQUAL(m->Radiometry.Bands[0], "XS1");
    BOOST_CHECK_EQUAL(m->Radiometry.Bands[1], "XS2");
    BOOST_CHECK_EQUAL(m->Radiometry.Bands[2], "XS3");
    BOOST_CHECK_EQUAL(m->Radiometry.Bands[3], "SWIR");

    BOOST_CHECK_EQUAL(m->Radiometry.Angles.PhiS, 145.43902353);
    BOOST_CHECK_EQUAL(m->Radiometry.Angles.ThetaS, 57.472591328);
    BOOST_CHECK_EQUAL(m->Radiometry.Angles.PhiV, -73.809703566);
    BOOST_CHECK_EQUAL(m->Radiometry.Angles.ThetaV, 18.141025097);
    BOOST_CHECK(std::isnan(m->Radiometry.Angles.Pitch));
    BOOST_CHECK(std::isnan(m->Radiometry.Angles.Roll));
}
