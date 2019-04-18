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
 
#include "CreateS2AnglesRaster.h"
#include "MetadataHelperFactory.h"

CreateS2AnglesRaster::CreateS2AnglesRaster()
{
}

void CreateS2AnglesRaster::DoInit( int res, std::string &xml)
{
    m_inXml = xml;
    m_nOutRes = res;
}

CreateS2AnglesRaster::OutputImageType::Pointer CreateS2AnglesRaster::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    std::unique_ptr<MetadataHelper<short>> pHelper = factory->GetMetadataHelper<short>(m_inXml);

    int nGridSize = pHelper->GetDetailedAnglesGridSize();
    if(nGridSize == 0) {
        itkExceptionMacro("Something is wrong. Grid size for angles is said to be 0!");
    }
    const std::vector<std::string> &resBandNames = pHelper->GetBandNamesForResolution(m_nOutRes);
    std::vector<int> resRelBandsIdxs;
    MetadataHelper<short>::VectorImageType::Pointer img = pHelper->GetImage(resBandNames, &resRelBandsIdxs, m_nOutRes);
    img->UpdateOutputInformation();
    auto sz = img->GetLargestPossibleRegion().GetSize();
    auto spacing = img->GetSpacing();

    int width = sz[0];
    int height = sz[1];

    if(width == 0 || height == 0) {
        itkExceptionMacro("The read width/height from the resolution metadata file is/are 0");
    }

    m_AnglesRaster = OutputImageType::New();
    OutputImageType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y

    OutputImageType::SizeType size;
    size[0]  = nGridSize;  // size along X
    size[1]  = nGridSize;  // size along Y

    OutputImageType::RegionType region;

    region.SetSize(size);
    region.SetIndex(start);

    auto viewingAngles = pHelper->GetDetailedViewingAngles(m_nOutRes);
    auto solarAngles = pHelper->GetDetailedSolarAngles();
    int nBandsForRes = pHelper->GetBandNamesForResolution(m_nOutRes).size();

    if((viewingAngles.size() == 0) || (viewingAngles.size() != (unsigned int)nBandsForRes) ||
            (solarAngles.Zenith.Values.size() == 0) || (solarAngles.Azimuth.Values.size() == 0)) {
        itkExceptionMacro("Mission does not have any detailed viewing or solar angles or viewing angles differ from the number of bands!");
    }

    if(solarAngles.Zenith.Values.size() != (unsigned int)nGridSize ||
        solarAngles.Azimuth.Values.size() != (unsigned int)nGridSize ) {
        itkExceptionMacro("The width and/or height of solar angles from the xml file is/are not as expected: "
                          << solarAngles.Azimuth.Values.size() << " or " <<
                          solarAngles.Azimuth.Values.size() << " instead " << nGridSize);
    }

    for (int band = 0; band < nBandsForRes; band++) {
        if(viewingAngles[band].Angles.Zenith.Values.size() != (unsigned int)nGridSize ||
            viewingAngles[band].Angles.Azimuth.Values.size() != (unsigned int)nGridSize )
            itkExceptionMacro("The width and/or height of computed angles from the xml file is/are not as expected: "
                              << viewingAngles[band].Angles.Zenith.Values.size() << " or " <<
                              viewingAngles[band].Angles.Azimuth.Values.size() << " instead " << nGridSize);
    }

    m_AnglesRaster->SetRegions(region);
    int nTotalAnglesNo = 2*(nBandsForRes+1);
    // we have 2 solar angles and 2 * N viewing angles
    m_AnglesRaster->SetNumberOfComponentsPerPixel(nTotalAnglesNo);
    OutputImageType::SpacingType anglesRasterSpacing;
    anglesRasterSpacing[0] = (((float)width) * spacing[0]) / nGridSize; // spacing along X
    anglesRasterSpacing[1] = (((float)height) * spacing[1]) / nGridSize; // spacing along Y
    m_AnglesRaster->SetSpacing(anglesRasterSpacing);
    m_AnglesRaster->Allocate();

    for(unsigned int i = 0; i < (unsigned int)nGridSize; i++) {
        for(unsigned int j = 0; j < (unsigned int)nGridSize; j++) {
            itk::VariableLengthVector<float> vct(nTotalAnglesNo);
            vct[0] = solarAngles.Zenith.Values[i][j];
            vct[1] = solarAngles.Azimuth.Values[i][j];
            for (int band = 0; band < nBandsForRes; band++) {
                vct[band * 2 + 2] = viewingAngles[band].Angles.Zenith.Values[i][j];
                vct[band * 2 + 3] = viewingAngles[band].Angles.Azimuth.Values[i][j];
            }

            OutputImageType::IndexType idx;
            idx[0] = j;
            idx[1] = i;
            m_AnglesRaster->SetPixel(idx, vct);
        }
    }
    m_AnglesRaster->UpdateOutputInformation();

    return m_ResampledBandsExtractor.getResampler(m_AnglesRaster, width, height)->GetOutput();
}
