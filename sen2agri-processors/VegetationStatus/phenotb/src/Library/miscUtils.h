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

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _MISCUTILS_H_
#define _MISCUTILS_H_

#include "phenoTypes.h"
#include <string>
#include <vector>

namespace pheno{
/** Create a gnuplot file for for plotting the true function yClean, the noisy
    one y, the estimation yHat as a function of the date vector t. xt are the true
    parameters, x the estimated ones and err the error.
*/
void generatePlotFile(const std::string& fileName, const VectorType& t, const VectorType& yClean, const VectorType& y, const VectorType& yHat, const VectorType& x, double err);
void generatePlotFile(const std::string& fileName, const std::string& title, const VectorType& t, const VectorType& y, const VectorType& yHat, const VectorType& x);

/// Split a string using a given separator
std::vector<std::string> string_split(const std::string& s, const std::string& sep);

/// return a string with a 0 at the front if the int is < 10
std::string pad_int(int x);
}
#endif
