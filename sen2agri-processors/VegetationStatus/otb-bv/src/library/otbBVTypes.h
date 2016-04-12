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
 
/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __OTBBVTYPES_H
#define __OTBBVTYPES_H

#include <map>

namespace otb
{
namespace Wrapper
{
namespace Tags
{
static const std::string BV="Biophysical Variables";
}
}

enum class IVNames {MLAI, ALA, CrownCover, HsD, N, Cab, Car, Cdm, CwRel, Cbp, 
                    Bs, IVNamesEnd};
enum AcquisitionParameters {TTS, TTO, PSI, TTS_FAPAR, AcquisitionParametersEnd};

using AcquisitionParsType = std::map< AcquisitionParameters, double >;
using PrecisionType = double;
using BVType = std::map< IVNames, PrecisionType >;

using NormalizationVectorType = 
  std::vector<std::pair<PrecisionType, PrecisionType>>;
}
#endif
