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

#define BOOST_TEST_MODULE MACCSRoundTrip
#include <boost/test/unit_test.hpp>

#include "otb_tinyxml.h"
#include "MACCSMetadataReader.hpp"
#include "MACCSMetadataWriter.hpp"

void testRoundTrip(const char *file)
{
    auto reader = itk::MACCSMetadataReader::New();
    auto writer = itk::MACCSMetadataWriter::New();

    TiXmlDocument doc;
    BOOST_REQUIRE(doc.LoadFile(file));

    TiXmlUnknown unk;
    std::istringstream("<?xml-stylesheet type=\"text/xsl\" href=\"DISPLAY/display.xsl\"?>") >> unk;
    doc.ReplaceChild(doc.FirstChild()->NextSibling(), unk);

    std::ostringstream original, ours;

    original << doc;
    auto m = reader->ReadMetadataXml(doc);
    BOOST_REQUIRE(m);
    ours << writer->CreateMetadataXml(*m);

    std::cerr << original.str() << '\n' << ours.str() << '\n';
    BOOST_CHECK_EQUAL(original.str(), ours.str());
}

BOOST_AUTO_TEST_CASE(MACCSRoundTrip)
{
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_CLD_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_CLD_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_QLT_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_QLT_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R2.HDR");
    testRoundTrip("S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");
    testRoundTrip("L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_ATB.HDR");
    testRoundTrip("L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_CLD.HDR");
    testRoundTrip("L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_MSK.HDR");
    testRoundTrip("L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_QLT.HDR");
    testRoundTrip("L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_FRE.HDR");
    testRoundTrip("L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_SRE.HDR");
    testRoundTrip("L8_TEST_L8C_PDTQLK_L2VALD_198030_20130626.HDR");
}
