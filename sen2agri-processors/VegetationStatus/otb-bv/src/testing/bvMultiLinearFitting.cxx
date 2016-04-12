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

#include "otbMultiLinearRegressionModel.h"

using MRM=otb::MultiLinearRegressionModel<double>;
MRM::MatrixType x_vec = {
  {0.0, 0},   
  {0.1, 0.01},
  {0.2, 0.04},
  {0.3, 0.09},
  {0.4, 0.16},
  {0.5, 0.25},
  {0.6, 0.36},
  {0.7, 0.49},
  {0.8, 0.64},
  {0.9, 0.81},
  {1.0, 1},   
  {1.1, 1.21},
  {1.2, 1.44},
  {1.3, 1.69},
  {1.4,  1.96 }};
MRM::VectorType w_vec = {0.1,    0.110517,0.12214,0.134986,0.149182,
                         0.164872,0.182212,0.201375,0.222554,0.24596,
                         0.271828,0.300417,0.332012,0.36693,0.40552};
MRM::VectorType y_vec = {1.01339,1.09543, 1.42592,1.44889, 1.64064, 1.4381,  
                         1.38541, 1.87696, 2.21684, 2.67938,2.71348, 
                         2.61466, 3.09834, 3.73597,4.39221};
MRM::VectorType best_fit = {1.09573, 0.424647, 1.15956};

int bvMultiLinearFitting(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }


  auto model = MRM::New();
  model->SetPredictorMatrix(x_vec);
  model->SetTargetVector(y_vec);
  model->SetWeightVector(w_vec);
  model->Train();
  auto result = model->GetModel();
  if(fabs(result[0]-best_fit[0])>0.01 ||
     fabs(result[1]-best_fit[1])>0.01 ||
     fabs(result[2]-best_fit[2])>0.01)
    return EXIT_FAILURE;

  MRM::VectorType test_vec = {x_vec[2][0], x_vec[2][1]};
  if(fabs(model->Predict(test_vec)-y_vec[2])>0.2)
    {
    std::cout << model->Predict(test_vec) << " " << y_vec[2] << " " 
              << fabs(model->Predict(test_vec)-y_vec[2]) << "\n";
    return EXIT_FAILURE;
    }
  model->Save("/tmp/mrr.txt");
  auto other_model = MRM::New();
  if(!other_model->CanReadFile("/tmp/mrr.txt"))
    {
    std::cout << "Could not read model file!\n";
    return EXIT_FAILURE;
    }
  other_model->Load("/tmp/mrr.txt");
  double diff{0};
  for(size_t i=0; i<model->GetModel().size(); i++)
    diff += fabs(model->GetModel()[i]-other_model->GetModel()[i]);
  if(diff>1e-6)
    {
    std::cout << "ERROR: Loaded model is different from saved model" << "\n";
    for(auto& v: model->GetModel())
      std::cout << v << " ";
    std::cout << "\n";
    for(auto& v: other_model->GetModel())
      std::cout << v << " ";
    std::cout << "\n";
    return EXIT_FAILURE;
    }
  if(fabs(model->Predict(test_vec)-other_model->Predict(test_vec))>1e-3)
    {
    std::cout << "Saved model gives different predictions "
              << model->Predict(test_vec) << " " 
              << other_model->Predict(test_vec) << " " 
              << fabs(model->Predict(test_vec)-other_model->Predict(test_vec)) 
              << "\n";
    return EXIT_FAILURE;
    }

  // check that a dummy model can't be read

  std::ofstream dummy_file;
  dummy_file.open("/tmp/dummy.txt", std::ofstream::out);
  dummy_file << "# NN regression model\n";
  dummy_file.close();
  auto dummy_model = MRM::New();
  if(dummy_model->CanReadFile("/tmp/dummy.txt"))
    {
    std::cout << "Error: abel to read a dummy model!\n";
    return EXIT_FAILURE;
    }

  
  return EXIT_SUCCESS;
}

int bvMultiLinearFittingConversions(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  auto inputListSample = MRM::InputListSampleType::New();
  auto outputListSample = MRM::TargetListSampleType::New();

  int nbInputVariables{2};
  inputListSample->SetMeasurementVectorSize(nbInputVariables);
  outputListSample->SetMeasurementVectorSize(1);

  for(size_t i=0; i<x_vec.size(); ++i)
    {
    MRM::TargetSampleType outputValue;
    outputValue[0] = y_vec[i];
    MRM::InputSampleType inputValue;
    inputValue.Reserve(nbInputVariables);
    for(auto var = 0; var < nbInputVariables; ++var)
      inputValue[var] = x_vec[i][var];
    inputListSample->PushBack(inputValue);
    outputListSample->PushBack(outputValue);
    }
  auto model = MRM::New();
  model->SetInputListSample(inputListSample);
  model->SetTargetListSample(outputListSample);

  auto x_out = model->GetPredictorMatrix();

  if(x_out != x_vec)
    {
    for(auto v : x_out)
      for(auto w : v)
        std::cout << w <<  " ";
    std::cout << std::endl;

    return EXIT_FAILURE;
    }
  auto y_out = model->GetTargetVector() ;

  if(y_out != y_vec)
    {
    for(auto v : y_out)
      std::cout << v <<  " ";
    std::cout << "-----------------" << std::endl;

    for(auto v : y_vec)
      std::cout << v <<  " ";
    std::cout << std::endl;

    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
