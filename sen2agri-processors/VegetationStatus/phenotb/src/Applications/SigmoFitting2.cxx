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

#define RESULT_BANDS_NO     6
namespace pheno
{
template <typename PixelType>
class SigmoFittingFunctor
{
protected:
  VectorType dv;

public:
  struct DifferentSizes {};
  SigmoFittingFunctor() {}

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
    auto princ_cycle = std::get<1>(approx);
    auto x_hat = std::get<0>(princ_cycle);
    auto min_max = std::get<1>(princ_cycle);
    auto A_hat = min_max.second - min_max.first;
    auto B_hat = min_max.first;

    for(int i = 0; i<4; i++) {
        result[i] = x_hat[i];
    }
    result[4] = A_hat;
    result[5] = B_hat;

     return result;
  }

  bool operator!=(const SigmoFittingFunctor a)
  {
    return (this->dates != a.dates) || (this->dv != a.dv) ;
  }

  bool operator==(const SigmoFittingFunctor a)
  {
    return !(*this == a);
  }

};
}

namespace otb
{
namespace Wrapper
{

class SigmoFitting2 : public Application
{
public:
/** Standard class typedefs. */
  typedef SigmoFitting2     Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  
/** Standard macro */
  itkNewMacro(Self);
  itkTypeMacro(SigmoFitting2, otb::Application);

  using FunctorType =
    pheno::SigmoFittingFunctor<FloatVectorImageType::PixelType>;
  using FilterType = pheno::BinaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                               FloatVectorImageType,
                                                               FunctorType>;

private:
  void DoInit()
  {
    SetName("SigmoFitting2");
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

  virtual ~SigmoFitting2()
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

OTB_APPLICATION_EXPORT(otb::Wrapper::SigmoFitting2)
