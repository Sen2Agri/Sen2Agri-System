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
#ifndef __OTBMLRM_H
#define __OTBMLRM_H

#include "itkMacro.h"
#include "gsl/gsl_multifit.h"
#include <vector>
#include "otbMachineLearningModel.h"

namespace otb{
template <typename PrecisionType=double>
class  MultiLinearRegressionModel :
    public MachineLearningModel<PrecisionType, PrecisionType>
{
public:
  using VectorType = std::vector<PrecisionType>;
  using MatrixType = std::vector<VectorType>;
  using TargetSampleType = typename
    MachineLearningModel<PrecisionType,
                         PrecisionType>::TargetSampleType;
  using InputSampleType = typename
    MachineLearningModel<PrecisionType,
                         PrecisionType>::InputSampleType;

  typedef itk::Statistics::ListSample<TargetSampleType> TargetListSampleType;
  typedef itk::Statistics::ListSample<InputSampleType> InputListSampleType;

  MultiLinearRegressionModel() : m_weights(false) {};

  /** Standard class typedefs. */
  typedef MultiLinearRegressionModel           Self;
  typedef MachineLearningModel<PrecisionType, PrecisionType> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro(MultiLinearRegressionModel, itk::MachineLearningModel);

  void SetPredictorMatrix(MatrixType x)
  {
    m_x = x;
  }
  void SetTargetVector(VectorType y)
  {
    m_y = y;
  }
  void SetWeightVector(VectorType w)
  {
    m_w = w;
    m_weights = true;
      }
  void Train()
  {
    this->multi_linear_fit();
  }

  void TrainClassification()
  {
  }

  void TrainRegression()
  {
  }

  PrecisionType Predict(const VectorType x) const
  {
    if(m_model.size()==0)
      {
      itkExceptionMacro(<< "Model is not initialized.")
        }
    if(x.size()!=(m_model.size()-1))
      {
      itkExceptionMacro(<< "Predictor vector and model have different sizes.")
        }

    PrecisionType result = m_model[0];
    for(size_t i=0; i<x.size(); i++)
      result += m_model[i+1]*x[i];

    return result;
  }

#if 0
  TargetSampleType Predict(const InputSampleType & input, double *quality = NULL) const ITK_OVERRIDE
  {
    VectorType tmp_vec(this->SampleToVector(input));
    TargetSampleType target;
    target[0] = this->Predict(tmp_vec);
    if (quality)
        *quality = 1.0;
    return target;
  }
#else
  TargetSampleType Predict(const InputSampleType & input) const ITK_OVERRIDE
  {
    VectorType tmp_vec(this->SampleToVector(input));
    TargetSampleType target;
    target[0] = this->Predict(tmp_vec);
    return target;
  }
#endif

  TargetSampleType PredictClassification(const InputSampleType & input) const
  {
    VectorType tmp_vec(this->SampleToVector(input));
    TargetSampleType target;
    target[0] = this->Predict(tmp_vec);
    return target;
  }

  TargetSampleType PredictRegression(const InputSampleType & input) const
  {
    VectorType tmp_vec(this->SampleToVector(input));
    TargetSampleType target;
    target[0] = this->Predict(tmp_vec);
    return target;
  }

  void SetInputListSample(InputListSampleType * ils)
  {
    MatrixType tmp_m;
    auto slIt = ils->Begin();
    auto slEnd = ils->End();
    while(slIt != slEnd)
      {
      tmp_m.push_back(this->SampleToVector(slIt.GetMeasurementVector()));
      ++slIt;
      }
    this->SetPredictorMatrix(tmp_m);
  }
  void SetTargetListSample(TargetListSampleType * tls)
  {
    VectorType tmp_v;
    auto slIt = tls->Begin();
    auto slEnd = tls->End();
    while(slIt != slEnd)
      {
      tmp_v.push_back(slIt.GetMeasurementVector()[0]);
      ++slIt;
      }
    this->SetTargetVector(tmp_v);

  }

  /** Save the model to file */
  void Save(const std::string & filename, const std::string & name="");

  /** Load the model from file */
  void Load(const std::string & filename, const std::string & name="");

  bool CanReadFile(const std::string &);

  bool CanWriteFile(const std::string &);

  VectorType GetModel()
  {
    return m_model;
  }

  MatrixType GetPredictorMatrix() const
  {
    return m_x;
  }

  VectorType GetTargetVector() const
  {
    return m_y;
  }

protected:
  void multi_linear_fit();

  std::string GetNameOfClass()
  {
    return std::string{"MultiLinearRegressionModel"};
  }

  template<typename MVType>
  VectorType SampleToVector(MVType mv) const
  {
    VectorType tmp_vec(mv.Size());
    for(size_t i=0; i<mv.Size(); ++i)
      tmp_vec[i] = mv[i];
    return tmp_vec;
  }
  MatrixType m_x;
  VectorType m_y;
  VectorType m_w;
  bool m_weights;
  VectorType m_model;

};
}//namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbMultiLinearRegressionModel.txx"
#endif

#endif
