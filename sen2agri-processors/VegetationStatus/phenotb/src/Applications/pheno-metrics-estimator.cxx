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

#include <fstream>
#include <random>

#include "itkMacro.h"
#include "phenoFunctions.h"
#include "dateUtils.h"
#include "miscUtils.h"

using VectorType = pheno::VectorType;
using PrecisionType = pheno::PrecisionType;
using SigmoParameterType = typename std::pair<VectorType, 
                                              std::pair<PrecisionType,
                                                        PrecisionType> >;
using SigmoMetricsType = std::tuple<PrecisionType, PrecisionType, 
                                    PrecisionType, PrecisionType, 
                                    PrecisionType, PrecisionType>;
using SimulationRecordType = std::tuple<SigmoParameterType,
                                        SigmoMetricsType,
                                        VectorType,
                                        VectorType>;

size_t count_dates(std::ifstream& inf)
{
  std::string headerline;
  std::getline(inf, headerline);
  /* The header line is like
    x0,x1,x2,x3,A,B,dgx0,t0,t1,t2,t3,dgx2,d1,d2,d3,d4,..,dn,p1,p2,p3,..,pn
    so for n dates there are 4+2+6+2*n values
  */
  size_t nbdates = pheno::string_split(headerline, ",").size();
  nbdates = (nbdates-(4+2+6))/2;
  return nbdates;
}

SimulationRecordType get_simulation_record(std::ifstream& inf, size_t nbdates)
{
  std::string recordline;
  std::getline(inf, recordline);
  /* The header line is like
    x0,x1,x2,x3,A,B,dgx0,t0,t1,t2,t3,dgx2,d1,d2,d3,d4,..,dn,p1,p2,p3,..,pn
  */
  auto record_values = pheno::string_split(recordline, ",");
  if(record_values.size() < 12) return SimulationRecordType{};

  VectorType sigmo_pars_x(4);
  sigmo_pars_x[0] = std::stod(record_values[0]);
  sigmo_pars_x[1] = std::stod(record_values[1]);
  sigmo_pars_x[2] = std::stod(record_values[2]);
  sigmo_pars_x[3] = std::stod(record_values[3]);
  std::pair<PrecisionType, PrecisionType>  sigmo_pars_AB = 
    std::make_pair(std::stod(record_values[4]), 
                   std::stod(record_values[5]));
  auto sigmo_pars = std::make_pair(sigmo_pars_x, sigmo_pars_AB);

  SigmoMetricsType sigmo_metrics = std::make_tuple(std::stod(record_values[6]),
                                                   std::stod(record_values[7]),
                                                   std::stod(record_values[8]),
                                                   std::stod(record_values[9]),
                                                   std::stod(record_values[10]),
                                                   std::stod(record_values[11]));
  VectorType date_vec(nbdates);
  VectorType profile_vec(nbdates);
  for(size_t i=0; i<nbdates; ++i)
    {
    date_vec[i] = std::stod(record_values[i+12]);
    profile_vec[i] = std::stod(record_values[i+12+nbdates]);
    }
  return std::make_tuple(sigmo_pars, sigmo_metrics, date_vec, profile_vec);
}

void write_estimation_result(std::ofstream& out_file, 
                             VectorType x, 
                             PrecisionType A, 
                             PrecisionType B, 
                             SigmoMetricsType m, 
                             VectorType x_hat, 
                             PrecisionType A_hat, 
                             PrecisionType B_hat, 
                             SigmoMetricsType m_hat)
{
  std::stringstream out_record{};
  out_record << x[0] << ",";
  out_record << x[1] << ",";
  out_record << x[2] << ",";
  out_record << x[3] << ",";
  out_record << A << ",";
  out_record << B << ",";
  out_record << std::get<0>(m) << ",";
  out_record << std::get<1>(m) << ",";
  out_record << std::get<2>(m) << ",";
  out_record << std::get<3>(m) << ",";
  out_record << std::get<4>(m) << ",";
  out_record << std::get<5>(m) << ",";

  out_record << x_hat[0] << ",";
  out_record << x_hat[1] << ",";
  out_record << x_hat[2] << ",";
  out_record << x_hat[3] << ",";
  out_record << A_hat << ",";
  out_record << B_hat << ",";
  out_record << std::get<0>(m_hat) << ",";
  out_record << std::get<1>(m_hat) << ",";
  out_record << std::get<2>(m_hat) << ",";
  out_record << std::get<3>(m_hat) << ",";
  out_record << std::get<4>(m_hat) << ",";
  out_record << std::get<5>(m_hat) << std::endl;

  out_file << out_record.str();
}

int main(int argc, char* argv[])
{
  if(argc != 3)
    {
    std::cout << "Usage: " << argv[0] << " in_filename out_filename \n";
    return EXIT_FAILURE;
    }

  std::string in_filename{argv[1]};
  std::string out_filename{argv[2]};

  std::ifstream in_file{in_filename};
  std::ofstream out_file{out_filename};
  out_file << "x0,x1,x2,x3,A,B,dgx0,t0,t1,t2,t3,dgx2,";
  out_file << "x0_hat,x1_hat,x2_hat,x3_hat,A_hat,B_hat," 
           << "dgx0_hat,t0_hat,t1_hat,t2_hat,t3_hat,dgx2_hat\n";

  size_t nbdates = count_dates(in_file);

  std::cout << "Profiles contain " << nbdates << " dates." << std::endl;
  
  while(in_file.good() && !in_file.eof())
    {
    // get the dates and the profile from the current line
    auto simulation_record = get_simulation_record(in_file, nbdates);
    if(simulation_record == SimulationRecordType{})
      {
      std::cout << "Empty record or last line.\n";
      continue;
      }
    auto x = std::get<0>(simulation_record).first;
    auto A = std::get<0>(simulation_record).second.first;
    auto B = std::get<0>(simulation_record).second.second;
    auto m = std::get<1>(simulation_record);

    auto date_vec = std::get<2>(simulation_record);
    auto profile_vec = std::get<3>(simulation_record);
    // estimate the sigmoid parameters
    auto approximation_result = 
      pheno::normalized_sigmoid::TwoCycleApproximation(profile_vec, date_vec);
    auto princ_cycle = std::get<1>(approximation_result);
    auto x_hat = std::get<0>(princ_cycle);
    auto min_max = std::get<1>(princ_cycle);
    auto A_hat = min_max.second - min_max.first;
    auto B_hat = min_max.first;
    //estimate the metrics
    auto m_hat = pheno::normalized_sigmoid::pheno_metrics<double>(x_hat);
    // write the real and estimated parameters and metrics
    write_estimation_result(out_file, x, A, B, m, x_hat, A_hat, B_hat, m_hat);
    }

  in_file.close();
  out_file.close();
    

  return EXIT_SUCCESS;
}
