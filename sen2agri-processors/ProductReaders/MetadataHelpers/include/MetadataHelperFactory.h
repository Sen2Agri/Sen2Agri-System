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
 
#ifndef COMPOSITENAMINGHELPERFACTORY_H
#define COMPOSITENAMINGHELPERFACTORY_H

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "MetadataHelper.h"
#include <vector>
#include <memory>

class MetadataHelperFactory : public itk::LightObject
{
public:
    typedef MetadataHelperFactory Self;
    typedef itk::LightObject Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(MetadataHelperFactory, itk::LightObject)

    template <typename PixelType, typename MasksPixelType=short>
    std::unique_ptr<MetadataHelper<PixelType, MasksPixelType>> GetMetadataHelper(const std::string& metadataFileName);
};

#endif // COMPOSITENAMINGHELPERFACTORY_H
