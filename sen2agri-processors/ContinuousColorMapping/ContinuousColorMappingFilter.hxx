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
 
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkUnaryFunctorImageFilter.h>

#include "ContinuousColorMappingFunctor.hxx"


typedef itk::UnaryFunctorImageFilter<otb::Wrapper::FloatVectorImageType,
                                     otb::Wrapper::UInt8RGBImageType,
                                     ContinuousColorMappingFunctor>
        ContinuousColorMappingFilter;
