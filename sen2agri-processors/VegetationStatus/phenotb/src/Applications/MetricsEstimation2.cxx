/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "itkBinaryFunctorImageFilter.h"

#include <vector>

#include "dateUtils.h"
#include "phenoFunctions.h"

using PrecisionType = double;

namespace otb
{
namespace Functor
{
}

#define RESULT_BANDS_NO     4
namespace Wrapper
{

template< class TInput1, class TInput2, class TOutput>
class MetricsEstimationFunctor2
{
public:
  MetricsEstimationFunctor2() {}
  ~MetricsEstimationFunctor2() {}
  bool operator!=(const MetricsEstimationFunctor2 &a) const
  {
      return false;
  }

  bool operator==(const MetricsEstimationFunctor2 & other) const
  {
    return !( *this != other );
  }

  inline TOutput operator()(const TInput1 & A) const
  {
    TOutput result(RESULT_BANDS_NO);
    int x_hat_size = A.Size()-2;
    std::vector<PrecisionType> x_hat(x_hat_size);
    result.Fill(NO_DATA);
    if(A.Size() != 6 || fabs(A[0] - NO_DATA) < EPSILON) {
        return result;
    }

    for(size_t i = 0; i < x_hat_size; i++) {
        x_hat[i] = A[i];
    }
    PrecisionType A_hat = A[4];
    PrecisionType B_hat = A[5];

    double dgx0, t0, t1, t2, t3, dgx2;
    std::tie(dgx0, t0, t1, t2, t3, dgx2) =
        pheno::normalized_sigmoid::pheno_metrics<double>(x_hat, A_hat, B_hat);

   result[0] = x_hat[0];
   result[1] = t0;
   result[2] = (t2-t1);
   result[3] = t3;

   return result;
  }
private:
};

class MetricsEstimation2 : public Application
{
public:
  /** Standard class typedefs. */
  typedef MetricsEstimation2               Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef float                                   PixelType;
  typedef FloatVectorImageType                    InputImageType;
  typedef FloatVectorImageType                    OutImageType;

  typedef MetricsEstimationFunctor2 <InputImageType::PixelType,
                                    InputImageType::PixelType,
                                    OutImageType::PixelType>                FunctorType;

  typedef itk::UnaryFunctorImageFilter<InputImageType,
                                        OutImageType, FunctorType> FilterType;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(MetricsEstimation2, otb::Application);

private:
  void DoInit()
  {

    SetName("MetricsEstimation2");
    SetDescription("Reprocess a BV time profile.");
   
    AddParameter(ParameterType_InputImage, "ipf", "Input profile file.");
    SetParameterDescription( "ipf", "Input file containing the souble logistic resulted parameters (x_hat (4 bands for x0, x1, x2 and x3), A_hat and B_hat)." );

    AddParameter(ParameterType_OutputImage, "opf", "Output profile file.");
    SetParameterDescription( "opf", "The raster containing the phenological parameters." );
  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
      FloatVectorImageType::Pointer ipf_image = this->GetParameterImage("ipf");
      unsigned int nb_ipf_bands = ipf_image->GetNumberOfComponentsPerPixel();
      if(nb_ipf_bands != 6) {
          itkExceptionMacro("Invalid number of parameters for phenological metrics=" <<
                            nb_ipf_bands << ", Expected = 6!!!");
      }


      //instantiate a functor with the regressor and pass it to the
      //unary functor image filter pass also the normalization values
      m_MetricsEstimationFilter = FilterType::New();

      m_MetricsEstimationFilter->SetFunctor(m_functor);
      m_MetricsEstimationFilter->SetInput(ipf_image);
      m_MetricsEstimationFilter->UpdateOutputInformation();
      m_MetricsEstimationFilter->GetOutput()->SetNumberOfComponentsPerPixel(RESULT_BANDS_NO);
      SetParameterOutputImage("opf", m_MetricsEstimationFilter->GetOutput());

  }

  FilterType::Pointer m_MetricsEstimationFilter;
  FunctorType m_functor;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::MetricsEstimation2)
