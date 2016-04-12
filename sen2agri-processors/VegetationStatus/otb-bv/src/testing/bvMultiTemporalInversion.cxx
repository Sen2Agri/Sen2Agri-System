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

#include "itkMacro.h"

#include "phenoFunctions.h"
#include "miscUtils.h"
#include "otbBVUtil.h"
#include "otbProSailSimulatorFunctor.h"
#include "otbMachineLearningModelFactory.h"
#include "otbNeuralNetworkRegressionMachineLearningModel.h"
#include "otbProfileReprocessing.h"

#include <vector>
#include <random>
#include <cassert>

using PrecisionType=double;
using VectorType=std::vector<PrecisionType>;
using SatRSRType = otb::SatelliteRSR<PrecisionType, PrecisionType>;
using ProSailType = otb::Functor::ProSailSimulator<SatRSRType>;
using SimulationType = typename ProSailType::OutputType;
using PixelType = ProSailType::OutputType;
typedef itk::VariableLengthVector<PrecisionType> InputSampleType;
typedef otb::NeuralNetworkRegressionMachineLearningModel<PrecisionType, 
                                                         PrecisionType> 
NeuralNetworkType;


std::pair<VectorType, VectorType> generate_lai(const VectorType doys)
{
  VectorType sigmo_pars{100, 3, 200, 3, 6, 0.1};
  VectorType simu_lai(doys.size());
  pheno::sigmoid::F(doys, sigmo_pars, simu_lai);
  auto noisy_lai = simu_lai;

  auto rng = std::mt19937(std::random_device{}());
  rng.seed(5);
  std::normal_distribution<> d(0.0,0.1);
  for(auto& l : noisy_lai)
    {
    l+=d(rng);
    l=(l<0)?0:l;
    }

  return std::make_pair(simu_lai, noisy_lai);

}

std::vector<PixelType> generate_reflectances(VectorType lai_vec, 
                                             std::string rsr_file,
                                             double solarzenith,
                                             double sensorzenith,
                                             double azimuth)
{
  auto satRSR = SatRSRType::New();
  short int nbBands = otb::countColumns(rsr_file.c_str())-2;
  satRSR->SetNbBands(nbBands);
  satRSR->SetSortBands(false);
  satRSR->Load(rsr_file.c_str());

  typename otb::AcquisitionParsType prosailPars;
  prosailPars[otb::TTS] = solarzenith;
  prosailPars[otb::TTO] = sensorzenith;
  prosailPars[otb::PSI] = azimuth;

  ProSailType prosail;
  prosail.SetRSR(satRSR);
  prosail.SetParameters(prosailPars);

  typename otb::BVType prosailBV;
  prosailBV[otb::IVNames::ALA] = 59.755;
  prosailBV[otb::IVNames::CrownCover] = 0.95768;
  prosailBV[otb::IVNames::HsD] = 0.18564;
  prosailBV[otb::IVNames::N] = 1.4942;
  prosailBV[otb::IVNames::Cab] = 64.632;
  prosailBV[otb::IVNames::Car] = 0;
  prosailBV[otb::IVNames::Cdm] = 0.0079628;
  prosailBV[otb::IVNames::CwRel] = 0.73298;
  prosailBV[otb::IVNames::Cbp] = 0.075167;
  prosailBV[otb::IVNames::Bs] = 0.72866;

  std::vector<PixelType> simus;
  auto rng = std::mt19937(std::random_device{}());
  rng.seed(1);
  std::normal_distribution<> d(0.0,0.05);
  for(auto l : lai_vec)
    {
    prosailBV[otb::IVNames::MLAI] = l;
    prosail.SetBVs(prosailBV);
    auto pix = prosail();
    //add noise to simulations
    for(auto& v : pix)
      v+=d(rng);
    simus.push_back(pix);
    }
  return simus;
}


int bvMultiTemporalInversion(int argc, char * argv[])
{

  double solarzenith = std::atof(argv[2]);
  double sensorzenith = std::atof(argv[3]);
  double azimuth = std::atof(argv[4]);
  std::string rsr_file{argv[1]};
  std::string bv_model_file{argv[5]};
  std::string error_model_file{argv[6]};
  VectorType doys;
  for(auto d=0; d<365; d+=10)
    doys.push_back(d);

  VectorType simu_lai, noisy_lai;

  std::tie(simu_lai, noisy_lai) = generate_lai(doys);
  auto simu_refls = generate_reflectances(noisy_lai, rsr_file, solarzenith,
                                          sensorzenith, azimuth);

  auto ndvi = noisy_lai;
  for(size_t i=0; i<simu_lai.size(); ++i)
    ndvi[i] = (simu_refls[i][3]-simu_refls[i][2])/
      (simu_refls[i][3]+simu_refls[i][2]+0.01);

  auto nn_regressor = NeuralNetworkType::New();
  nn_regressor->Load(bv_model_file);

  auto error_regressor = NeuralNetworkType::New();
  error_regressor->Load(error_model_file);

  VectorType estim_lai;
  VectorType estim_error;
  VectorType msks = VectorType(simu_lai.size(), 4);
  for(size_t i=0; i<simu_lai.size(); ++i)
    {
    InputSampleType pix(simu_refls[i].data(), simu_refls[i].size()-2);
    estim_lai.push_back(nn_regressor->Predict(pix)[0]);
    estim_error.push_back(error_regressor->Predict(pix)[0]);
    }

  VectorType smooth_lai{};
  VectorType out_flag_vec{};

  size_t bwr = 1;
  size_t fwr = 1;
  if(argc==10)
    {
    bwr = std::stoi(argv[8]);
    fwr = std::stoi(argv[9]);
    std::cout << "Radiuses: " << bwr << " " << fwr << std::endl;
    }
  std::tie(smooth_lai, out_flag_vec) = 
    otb::smooth_time_series_local_window_with_error(doys,
                                                    estim_lai, 
                                                    estim_error,
                                                    msks,
                                                    bwr,
                                                    fwr);

  std::ofstream res_file;
  res_file.open(argv[7]);
  for(size_t i=0; i<simu_lai.size(); ++i)
    res_file << doys[i] << " " << noisy_lai[i] << " " << estim_lai[i] << " " 
             << estim_error[i] << " "
             << smooth_lai[i]  << " " 
             << ndvi[i] << std::endl;
  res_file.close();
  return EXIT_SUCCESS;
}

int bvMultiTemporalInversionFromFile(int argc, char * argv[])
{
  if(argc!=5)
    {
    std::cout << "Usage: " << argv[0] << " data_file bwradius fwradius tolerance" << std::endl;
    return EXIT_FAILURE;
    }
  std::string fname{argv[1]};
  size_t bwr = std::stoi(argv[2]);
  size_t fwr = std::stoi(argv[3]);
  double tolerance = std::stod(argv[4]);
  std::ifstream dataFile(fname);
  if(!dataFile)
    itkGenericExceptionMacro(<< "Could not open file " << fname << "\n");
  VectorType doys{};
  VectorType estim_lai{};
  VectorType estim_error{};
  VectorType smooth_lai_ref{};
  std::string line;
  //read the header line
  std::getline(dataFile, line);
  while(dataFile.good())
    {
    std::getline(dataFile, line);
    auto tokens = pheno::string_split(line, " ");
    if(tokens.size() > 5)
      {
      doys.push_back(std::stod(tokens[0]));
      estim_lai.push_back(std::stod(tokens[2]));
      estim_error.push_back(std::stod(tokens[3]));
      smooth_lai_ref.push_back(std::stod(tokens[4]));
      }
    }
  VectorType msks = VectorType(estim_lai.size(), 4);
  VectorType smooth_lai{};
  VectorType out_flag_vec{};
  std::tie(smooth_lai, out_flag_vec) = 
    otb::smooth_time_series_local_window_with_error(doys,
                                                    estim_lai, 
                                                    estim_error, 
                                                    msks,
                                                    bwr, fwr);
  double err{0.0};
  for(size_t i=0; i<smooth_lai.size(); ++i)
    {
    err += fabs(smooth_lai[i]-smooth_lai_ref[i]);
    }
  err /= smooth_lai.size();
  if(err > tolerance)
    {
    std::cout << "error = " << err << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
