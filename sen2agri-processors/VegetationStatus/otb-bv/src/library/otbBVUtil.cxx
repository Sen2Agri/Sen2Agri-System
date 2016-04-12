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
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "itkMacro.h"

namespace otb
{
size_t countColumns(std::string fileName)
{
  std::ifstream ifile(fileName.c_str());
  std::string line;
  size_t nbSpaces = 0;
  if (ifile.is_open())
    {
    getline(ifile,line);
    ifile.close();
    boost::trim(line);
    auto found = line.find(' ');
    while(found!=std::string::npos)
      {
      ++nbSpaces;
      found = line.find(' ', found+1);
      }
    return nbSpaces+1;
    }
  else
    {
    itkGenericExceptionMacro(<< "Could not open file " << fileName);
    }

}
}

