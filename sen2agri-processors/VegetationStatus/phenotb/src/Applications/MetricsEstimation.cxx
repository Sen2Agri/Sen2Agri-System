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
using VectorType = std::vector<PrecisionType>;
constexpr PrecisionType not_processed_value{0};
constexpr PrecisionType processed_value{1};

namespace otb
{
namespace Functor
{
}


namespace Wrapper
{

template< class TInput1, class TInput2, class TOutput>
class MetricsEstimationFunctor
{
public:
  MetricsEstimationFunctor() {}
  ~MetricsEstimationFunctor() {}
  bool operator!=(const MetricsEstimationFunctor &a) const
  {
      return (this->date_vect != a.date_vect);
  }

  bool operator==(const MetricsEstimationFunctor & other) const
  {
    return !( *this != other );
  }

  void SetDates(VectorType &idDates)
  {
      date_vect = idDates;
  }

  inline TOutput operator()(const TInput1 & A) const
  {
      //itk::VariableLengthVector vect;


    TOutput result(6);    
    TInput1 A2(A.Size());
    result.Fill(NO_DATA);
    size_t j = 0;
    for(size_t i = 0; i < A.Size(); i++) {
        if(fabs(A[i] - NO_DATA) >= EPSILON) {
        //if(A[i] >= 0) {
            A2[j++] = A[i] /*/ 10000*/;
        }
    }

    if(j == 0)
        return result;
    if(j != A.Size())
        A2.SetSize(j, false);


     // TODO: Uncomment this if the approximation is needed and use in pheno_metrix x_hat instead of ts
    int nbBvElems = A2.GetNumberOfElements();

    vnl_vector<double> ts, dv;
    for(size_t i = 0; i<nbBvElems; i++) {
        ts[i] = A2[i];
    }
    for(size_t i = 0; i<date_vect.size(); i++) {
        dv[i] = date_vect[i];
    }

    auto approximation_result =
      pheno::normalized_sigmoid::TwoCycleApproximation(ts, dv);
    auto princ_cycle = std::get<1>(approximation_result);
    auto x_hat = std::get<0>(princ_cycle);

    double dgx0, t0, t1, t2, t3, dgx2;
    std::tie(dgx0, t0, t1, t2, t3, dgx2) =
        pheno::normalized_sigmoid::pheno_metrics<double>(x_hat);

    result[0] = dgx0;
    result[1] = t0;
    result[2] = t1;
    result[3] = t2;
    result[4] = t3;
    result[5] = dgx2;


    return result;
  }
private:
  // input dates vector
  VectorType date_vect;
};

class MetricsEstimation : public Application
{
public:
  /** Standard class typedefs. */
  typedef MetricsEstimation               Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef float                                   PixelType;
  typedef FloatVectorImageType                    InputImageType;
  typedef FloatVectorImageType                    OutImageType;

  typedef MetricsEstimationFunctor <InputImageType::PixelType,
                                    InputImageType::PixelType,
                                    OutImageType::PixelType>                FunctorType;

  typedef itk::UnaryFunctorImageFilter<InputImageType,
                                        OutImageType, FunctorType> FilterType;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(MetricsEstimation, otb::Application);

private:
  void DoInit()
  {

    SetName("MetricsEstimation");
    SetDescription("Reprocess a BV time profile.");
   
    AddParameter(ParameterType_InputImage, "ipf", "Input profile file.");
    SetParameterDescription( "ipf", "Input file containing the profile to process. This is an ASCII file where each line contains the date (YYYMMDD) the BV estimation and the error." );

    AddParameter(ParameterType_InputFilename, "indates", "The file containing the dates of each image acquisition, each date being in the format yyyymmdd");

    AddParameter(ParameterType_OutputImage, "opf", "Output profile file.");
    SetParameterDescription( "opf", "Filename where the reprocessed profile saved. This is an raster band contains the new BV estimation value for each pixel. The last band contains the boolean information which is 0 if the value has not been reprocessed." );
  }

  void DoUpdateParameters()
  {
    //std::cout << "MetricsEstimation::DoUpdateParameters" << std::endl;
  }

  void DoExecute()
  {
      FloatVectorImageType::Pointer ipf_image = this->GetParameterImage("ipf");
      unsigned int nb_ipf_bands = ipf_image->GetNumberOfComponentsPerPixel();
      std::string datesFileName = GetParameterString("indates");
      std::ifstream datesFile;
      datesFile.open(datesFileName);
      if (!datesFile.is_open()) {
          itkExceptionMacro("Can't open dates file for reading!");
      }

      // read the file and save the dates as second from Epoch to a vector
      std::vector<struct tm> dates;
      VectorType inDates;
      std::string value;      
      while (std::getline(datesFile, value)) {
          struct tm tmDate = {};
          if (strptime(value.c_str(), "%Y%m%d", &tmDate) == NULL) {
              itkExceptionMacro("Invalid value for a date: " + value);
          }
          dates.emplace_back(tmDate);
//          inDates.push_back(mktime(&tmDate) / 86400);
      }

      const auto &days = pheno::tm_to_doy_list(dates);
      inDates.resize(days.size());
      std::copy(std::begin(days), std::end(days), std::begin(inDates));

      // close the file
      datesFile.close();

//      if (!inDates.empty())
//      {
//          auto min = *std::min_element(inDates.begin(), inDates.end());
//          std::transform(inDates.begin(), inDates.end(), inDates.begin(),
//                         [=](PrecisionType v) { return v - min; });
//      }

      if((nb_ipf_bands == 0) || (nb_ipf_bands != inDates.size())) {
          itkExceptionMacro("Invalid number of bands or number of dates: ipf bands=" <<
                            nb_ipf_bands << ", nb_xmls=" << inDates.size());
      }


      //instantiate a functor with the regressor and pass it to the
      //unary functor image filter pass also the normalization values
      m_MetricsEstimationFilter = FilterType::New();
      m_functor.SetDates(inDates);
      m_MetricsEstimationFilter->SetNumberOfThreads(1);

      m_MetricsEstimationFilter->SetFunctor(m_functor);
      m_MetricsEstimationFilter->SetInput(ipf_image);
      m_MetricsEstimationFilter->UpdateOutputInformation();
      m_MetricsEstimationFilter->GetOutput()->SetNumberOfComponentsPerPixel(6);
      SetParameterOutputImage("opf", m_MetricsEstimationFilter->GetOutput());

  }

  FilterType::Pointer m_MetricsEstimationFilter;
  FunctorType m_functor;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::MetricsEstimation)
