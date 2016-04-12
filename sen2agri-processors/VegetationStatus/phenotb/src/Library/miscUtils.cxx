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
#include "miscUtils.h"
#include "itkMacro.h"
#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>

namespace pheno{
void generatePlotFile(const std::string& fileName, const VectorType& t, const VectorType& yClean, const VectorType& y, const VectorType& yHat, const VectorType& x, double err)
{
  std::ofstream plotFile;
  
  try
    {
    plotFile.open(fileName.c_str(), std::ofstream::out);
    }
  catch(...)
    {
    itkGenericExceptionMacro(<< "Could not open file " << fileName);
    }

//  plotFile << std::setprecision(4);

  plotFile << "#set title \"";
  for(auto & elem : x)
    {
    plotFile << elem << " ";
    }
  plotFile << "e=" << err; 
  plotFile << "\"" << std::endl;
  plotFile << "#plot \"" << fileName << "\" "<< " using 1:2 with lines ";
  plotFile << "title \"Ideal\", "; 
  plotFile << "\"" << fileName << "\" "<< " using 1:3 with lines ";
  plotFile << "title \"Real\", "; 
  plotFile << "\"" << fileName << "\" "<< " using 1:4 with lines ";
  plotFile << "title \"Approx\" ";
  plotFile << std::endl;
  
  for(size_t i = 0; i < t.size(); i++)
    {
    plotFile << std::left << t[i] << "\t";
    plotFile << std::left << yClean[i] << "\t";
    plotFile << std::left << y[i] << "\t";
    plotFile << std::left << yHat[i] << std::endl;
    }
}

void generatePlotFile(const std::string& fileName, const std::string& title, const VectorType& t, const VectorType& y, const VectorType& yHat, const VectorType& x)
{
  std::ofstream plotFile;
  
  try
    {
    plotFile.open(fileName.c_str(), std::ofstream::out);
    }
  catch(...)
    {
    itkGenericExceptionMacro(<< "Could not open file " << fileName);
    }

  auto minmax = std::minmax_element(std::begin(y), std::end(y));

  auto vmax = *(minmax.second);
  auto vmin = *(minmax.first);


//  plotFile << std::setprecision(4);

  plotFile << "#set title \"";
  plotFile << title; 
  plotFile << "\"" << std::endl;
  plotFile << "#plot \"" << fileName << "\" "<< " using 1:2 with linespoints ";
  plotFile << "title \"Real\", "; 
  plotFile << "\"" << fileName << "\" "<< " using 1:3 with points ";
  plotFile << "title \"Fit\", ";
  plotFile << "(1.0/(1+exp(("<< x[0] <<"-x)/"<< x[1] <<"))-1.0/(1+exp(("<< x[2] <<"-x)/"<< x[3] <<")))*(" << vmax << "-" << vmin << ")+" << vmin << " ";
  plotFile << "title \"Approx\" ";
  plotFile << std::endl;
  
  for(size_t i = 0; i < t.size(); i++)
    {
    plotFile << std::left << t[i] << "\t";
    plotFile << std::left << y[i] << "\t";
    plotFile << std::left << yHat[i] << std::endl;
    }

  plotFile << "#{" << std::endl;
  plotFile << "#\t{X, Y}," << std::endl;
  plotFile << "#\t{" << t[0];
  for(size_t i = 1; i < t.size(); i++) plotFile << ", " << t[i];
  plotFile << "}," << std::endl;
  plotFile << "#\t{" << y[0];
  for(size_t i = 1; i < y.size(); i++) plotFile << ", " << y[i];
  plotFile << "}," << std::endl;
  plotFile << "#\t{" << x[0];
  for(size_t i = 1; i < x.size(); i++) plotFile << ", " << x[i];
  plotFile << "}" << std::endl;
  plotFile << "#}," << std::endl;

}

std::vector<std::string> string_split(const std::string& s, const std::string& sep)
{
  std::vector<std::string> res;
  boost::split(res, s, boost::is_any_of(sep));
  return res;
}

std::string pad_int(int x)
{
  return (x<10?"0":"")+std::to_string(x);
}
}
