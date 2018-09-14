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
 
#include "MetadataHelperFactory.h"
#include "Spot4MetadataHelper.h"
#include "MACCSMetadataHelper.h"

std::unique_ptr<MetadataHelper> MetadataHelperFactory::GetMetadataHelper(const std::string& metadataFileName, int nResolution)
{
    std::unique_ptr<MetadataHelper> spot4MetadataHelper(new Spot4MetadataHelper);
    if (spot4MetadataHelper->LoadMetadataFile(metadataFileName, nResolution))
        return spot4MetadataHelper;

    std::unique_ptr<MetadataHelper> maccsMetadataHelper(new MACCSMetadataHelper);
    if (maccsMetadataHelper->LoadMetadataFile(metadataFileName, nResolution))
        return maccsMetadataHelper;

    itkExceptionMacro("Unable to read metadata from " << metadataFileName);

    return NULL;
}
