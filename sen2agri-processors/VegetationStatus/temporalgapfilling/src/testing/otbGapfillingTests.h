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

  Program:   gapfilling
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See gapfilling-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkMacro.h"

#include "itkVariableLengthVector.h"
#include "gapfilling.h"
#include "phenoFunctions.h"
#include <tuple>

using ValueType = double;
using PixelType = itk::VariableLengthVector<ValueType>;
using VectorType = std::vector<ValueType>;
using TestDataType = std::vector<std::tuple<VectorType, VectorType, VectorType, VectorType, VectorType> >;
using MCTestDataType = std::vector<std::tuple<VectorType, VectorType, VectorType> >; 
using MCTestOutDatesDataType = std::vector<std::tuple<VectorType, VectorType, VectorType, VectorType> >; 
