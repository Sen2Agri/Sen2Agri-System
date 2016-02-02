/*=========================================================================
  Program:   phenotb
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperChoiceParameter.h"

#include "phenoFunctions.h"

/*std::vector<float> pix_dummy = {
    251,
    200,
    213,
    4066,
    2456,
    4173,
    5986,
    8232,
    9017,
    9273,
    9428,
    2806,
    3556,
    5145,
    8547,
    8646,
    5639
};

std::vector<int> msk_dummy ={
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    0
};
std::vector<int> days1 = {37,
                          57,
                          77,
                          92,
                          102,
                          107,
                          112,
                          117,
                          122,
                          127,
                          132,
                          137,
                          147,
                          152,
                          157,
                          162,
                          167
};
*/

// we have 4 phenological parameters and 1 band for the flags
#define RESULT_BANDS_NO     5
namespace pheno
{
template <typename PixelType>
class PhenologicalNDVIMeticsFunctor
{
protected:
  VectorType dv;

public:
  struct DifferentSizes {};
  PhenologicalNDVIMeticsFunctor() {};

  void SetDates(const std::vector<tm>& d) {
    dv = VectorType{static_cast<unsigned int>(d.size())};
    const auto &days = pheno::tm_to_doy_list(d);
    std::copy(std::begin(days), std::end(days), std::begin(dv));
  }

  // dates is already given in doy
  // valid pixel has a mask==0
  PixelType operator()(PixelType pix, PixelType mask)
  {
    auto nbDates = pix.GetSize();

    if(dv.size()!=nbDates) throw DifferentSizes{};

    PixelType tmpix{nbDates};
    tmpix.Fill(typename PixelType::ValueType{0});

    VectorType vec(nbDates);
    for(size_t i=0; i<nbDates; i++)
      {
      vec[i] = pix[i] / 10000;
      }


    // A date is valid if it is not NaN and the mask value == 0.
    auto pred = [=](int e) { return !(std::isnan(vec[e])) && (vec[e] >= 0) &&
                             (mask[e]==(typename PixelType::ValueType{0})); };
    auto f_profiles = filter_profile_fast(vec, dv, pred);

    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;

    // If there are not enough valid dates, keep the original value
    PixelType result(RESULT_BANDS_NO);
    result.Fill(NO_DATA);
    if(profile.size() < 4)
      {
      return result;
      }

    auto approx = normalized_sigmoid::TwoCycleApproximation(profile, t);
/*    auto x_1 = std::get<0>(std::get<1>(approx));
    auto mm1 = std::get<1>(std::get<1>(approx));
    auto x_2 = std::get<0>(std::get<2>(approx));
    auto mm2 = std::get<1>(std::get<2>(approx));
*/
    auto princ_cycle = std::get<1>(approx);
    auto x_hat = std::get<0>(princ_cycle);
    auto min_max = std::get<1>(princ_cycle);
    auto A_hat = min_max.second - min_max.first;
    auto B_hat = min_max.first;

/* Debug only
    auto x0_hat = x_hat[0];
    auto x1_hat = x_hat[1];
    auto x2_hat = x_hat[2];
    auto x3_hat = x_hat[3];
*/

      // The result uses either the original or the approximated value depending on the mask value
//      PixelType result(nbDates);
//      VectorType tmps1(nbDates);
//      VectorType tmps2(nbDates);

//      // Compute the approximation
//      normalized_sigmoid::F<VectorType>()(dv, x_1, tmps1);
//      normalized_sigmoid::F<VectorType>()(dv, x_2, tmps2);

//      normalized_sigmoid::F<VectorType>()(dv, x_hat, tmps1);
//      auto tmpres = tmps1*(mm1.second-mm1.first)+mm1.first
//        + tmps2*(mm2.second-mm2.first)+mm2.first;

//      double dgx0, t0, t1, t2, t3, dgx2;
//      std::tie(dgx0, t0, t1, t2, t3, dgx2) =
//          pheno::normalized_sigmoid::pheno_metrics<double>(tmpres);

     double dgx0, t0, t1, t2, t3, dgx2;
     std::tie(dgx0, t0, t1, t2, t3, dgx2) =
         pheno::normalized_sigmoid::pheno_metrics<double>(x_hat, A_hat, B_hat);

    result[0] = x_hat[0];
    result[1] = t0;
    result[2] = (t2-t1);
    result[3] = t3;
    // add also the number of valid dates that were used in the processing
    result[4] = profile.size();

      /*result[0] = dgx0;
      result[1] = t0;
      result[2] = t1;
      result[3] = t2;
      result[4] = t3;
      result[5] = dgx2;
      */

      return result;
  }

  bool operator!=(const PhenologicalNDVIMeticsFunctor a)
  {
    return (this->dates != a.dates) || (this->dv != a.dv) ;
  }

  bool operator==(const PhenologicalNDVIMeticsFunctor a)
  {
    return !(*this == a);
  }

};
}

namespace otb
{
namespace Wrapper
{

class PhenologicalNDVIMetrics : public Application
{
public:
/** Standard class typedefs. */
  typedef PhenologicalNDVIMetrics     Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  
/** Standard macro */
  itkNewMacro(Self);
  itkTypeMacro(PhenologicalNDVIMetrics, otb::Application);

  using FunctorType =
    pheno::PhenologicalNDVIMeticsFunctor<FloatVectorImageType::PixelType>;
  using FilterType = pheno::BinaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                               FloatVectorImageType,
                                                               FunctorType>;


private:
  void DoInit()
  {
    SetName("PhenologicalNDVIMetrics");
    SetDescription("");

    // Documentation
    SetDocName("Double sigmoid fitting");
    SetDocLongDescription("");
    SetDocLimitations("None");
    SetDocAuthors("Jordi Inglada");
    
    AddParameter(ParameterType_InputImage,  "in",   "Input Image");
    SetParameterDescription("in", "Input image");
    MandatoryOn("in");

    AddParameter(ParameterType_InputImage,  "mask",   "Mask Image");
    SetParameterDescription("mask", "Input validity mask");
    MandatoryOff("mask");

    AddParameter(ParameterType_String, "dates", "Date File");
    SetParameterDescription("dates", "Name of the file containing the dates for the images");
    MandatoryOn("dates");

    AddParameter(ParameterType_String, "mode", "Processing mode: profile or params");
    SetParameterDescription("mode", "Choose between producing as output the reprocessed profile or the parametres of the two-cycle sigmoid. Default value is profile.");
    MandatoryOff("mode");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription("out", "Output image");
    MandatoryOn("out");
    
    AddRAMParameter();
  }

  virtual ~PhenologicalNDVIMetrics()
  {
  }


  void DoUpdateParameters() 
  {
    // Nothing to do here : all parameters are independent
  }


  void DoExecute() 
  {

    // prepare the vector of dates
    auto dates = pheno::parse_date_file(this->GetParameterString("dates"));
    // pipeline
    FloatVectorImageType::Pointer inputImage = this->GetParameterImage("in");
    FloatVectorImageType::Pointer maskImage = this->GetParameterImage("mask");
    inputImage->UpdateOutputInformation();
    maskImage->UpdateOutputInformation();
    filter = FilterType::New();

    filter->SetInput(0, inputImage);
    filter->SetInput(1, maskImage);
    filter->GetFunctor().SetDates(dates);
    filter->SetNumberOfOutputBands(RESULT_BANDS_NO);
    filter->UpdateOutputInformation();
    SetParameterOutputImage("out", filter->GetOutput());
  }

  FilterType::Pointer filter;

};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::PhenologicalNDVIMetrics)
