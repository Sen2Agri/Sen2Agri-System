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

VectorType generate_doy_vector(size_t nb_dates)
{
  auto m_RNG = std::mt19937(std::random_device{}());
  std::uniform_int_distribution<> d(1, 365);
  std::vector<PrecisionType> doys{};
  for(size_t i=0; i<nb_dates; ++i)
    {
    doys.push_back(static_cast<PrecisionType>(d(m_RNG)));
    }
  std::sort(std::begin(doys), std::end(doys));
  VectorType res(nb_dates, nb_dates, doys.data());
  return res;
}

/** Retuns a pair where the first is the x vector of the 4 nomalized sigmoid
    parameters, and the second is a pair containing the amplitude A and the
    minimum value B.
*/
SigmoParameterType 
generate_double_sigmo_parameters(PrecisionType max_val)
{
  auto m_RNG = std::mt19937(std::random_device{}());
  VectorType x(4);
  {
  std::normal_distribution<> d(365/4,60);
  x[0] = d(m_RNG);
  }
  {
  std::normal_distribution<> d(7,4);
  x[1] = d(m_RNG);
  }
  {
  std::normal_distribution<> d(70,30);
  x[2] = x[0]+d(m_RNG);
  }
  {
  std::normal_distribution<> d(7,4);
  x[3] = d(m_RNG);
  }
  PrecisionType A = max_val;
  PrecisionType B{0};
  {
  PrecisionType stdev = max_val/100.0;
  std::normal_distribution<> d(0,stdev);
  A = (max_val-3*stdev)+d(m_RNG);
  B = (3*stdev)+d(m_RNG);
  }
  return std::make_pair(x, std::make_pair(A,B));
}

VectorType simulate_noisy_profile(VectorType dates, VectorType x, PrecisionType A, 
                                  PrecisionType B, PrecisionType noise_std)
{
  VectorType p(dates.size());
  pheno::normalized_sigmoid::F<VectorType>()(dates, x, p);
  auto m_RNG = std::mt19937(std::random_device{}());
  std::normal_distribution<> d(0, noise_std);
  for(auto& v : p)
    v = A*v + B + d(m_RNG);
  return p;
}

void write_simulation(std::ofstream& out_file, SigmoParameterType sp, 
                      SigmoMetricsType m, VectorType dates, 
                      VectorType p)
{
  out_file << sp.first[0] << ","
           << sp.first[1] << ","
           << sp.first[2] << ","
           << sp.first[3] << ","
           << sp.second.first << ","
           << sp.second.second << ","
           << std::get<0>(m) << ","
           << std::get<1>(m) << ","
           << std::get<2>(m) << ","
           << std::get<3>(m) << ","
           << std::get<4>(m) << ","
           << std::get<5>(m);
  for(auto d : dates)
    out_file << "," << d;
  for(auto v : p)
    out_file << "," << v;
 
  out_file << "\n";
}

int main(int argc, char* argv[])
{
  if(argc != 6)
    {
    std::cout << "Usage: " << argv[0] 
              << " max_val nb_dates nb_profiles noise_std out_filename \n";
    return EXIT_FAILURE;
    }
  double max_val = std::atof(argv[1]);
  size_t nb_dates = std::atoi(argv[2]);
  size_t nb_profiles = std::atoi(argv[3]);
  double noise_std = std::atof(argv[4]);
  std::string out_filename{argv[5]};

  std::ofstream out_file{out_filename};
  out_file << "x0,x1,x2,x3,A,B,dgx0,t0,t1,t2,t3,dgx2";
  for(size_t i=0; i<nb_dates; ++i)
    out_file << ",d" << i+1;
  for(size_t i=0; i<nb_dates; ++i)
    out_file << ",p" << i+1;
  out_file << "\n";
  for(size_t i=0; i<nb_profiles; ++i)
    {
    auto dates = generate_doy_vector(nb_dates);
    auto sp = generate_double_sigmo_parameters(max_val);
    auto m = pheno::normalized_sigmoid::pheno_metrics<double>(sp.first);
    auto p = simulate_noisy_profile(dates, sp.first, sp.second.first, 
                                    sp.second.second, noise_std);
    write_simulation(out_file, sp, m, dates, p);
    }

  out_file.close();
    

  return EXIT_SUCCESS;
}
