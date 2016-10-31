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
 
#ifndef CREATE_S2_ANGLES_RASTER_H
#define CREATE_S2_ANGLES_RASTER_H

#include "otbWrapperTypes.h"
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "ViewingAngles.hpp"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"

#include "ImageResampler.h"

class CreateS2AnglesRaster
{
public:
    typedef otb::Wrapper::FloatVectorImageType                  OutputImageType;
    typedef otb::ImageFileReader<otb::Wrapper::Int16VectorImageType>      ImageReaderType;

public:
    CreateS2AnglesRaster();
    void DoInit(int res, std::string &xml);
    OutputImageType::Pointer DoExecute();
    const char * GetNameOfClass() { return "CreateS2AnglesRaster"; }

private:
    OutputImageType::Pointer            m_AnglesRaster;
    std::string                         m_inXml;
    int                                 m_nOutRes;
    ImageResampler<OutputImageType, OutputImageType> m_ResampledBandsExtractor;

};

#endif // CREATE_S2_ANGLES_RASTER_H
