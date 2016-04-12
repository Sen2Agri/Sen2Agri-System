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
#ifndef _PHENOFUNCTIONS_H_
#define _PHENOFUNCTIONS_H_

#include <vnl/vnl_least_squares_function.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/vnl_transpose.h>
#include <functional>
#include <algorithm>
#include "itkVariableLengthVector.h"
#include "phenoTypes.h"
#include "dateUtils.h"
#include "itkBinaryFunctorImageFilter.h"

#define NO_DATA -10000.0f
#define EPSILON 0.0001f




namespace pheno
{
template<ContainerC V>
using FilterType = std::function<double (V, const V&)>;

using MinMaxType = std::pair<double, double>;

using CoefficientType = VectorType;

using ApproximationErrorType = double;

using ApproximationResultType = std::tuple<CoefficientType, MinMaxType, VectorType, VectorType,
                                           ApproximationErrorType>;

template<typename FunctionType>
class ParameterCostFunction : public vnl_least_squares_function
{
public:
  ParameterCostFunction(unsigned int nbPars, unsigned int nbD, 
                        const VectorType& yy, const VectorType& tt, 
                        FunctionType func) :
    vnl_least_squares_function(nbPars, nbD, no_gradient), y(yy), t(tt), 
    nbDates(nbD), phenoFunction(std::move(func)) {}

  inline
  void f(const VectorType& x, VectorType& fx) override
  {
    phenoFunction(t, x, fx);
    for(size_t i=0; i<nbDates; ++i)
      fx[i] = y[i] - fx[i];
  }

private:
  VectorType y;
  VectorType t;
  unsigned int nbDates;
  FunctionType phenoFunction;
};

/** Gaussian weighting of a vector. The gaussian is centered on the maximum of the profile
    and its stdev is estimated by tresholding the vector at max*thres.
*/
double gaussianFunction(double x, double m, double s);
template <ContainerC V>
V gaussianWeighting(const V& p, const V& t, double m, double s);
template <ContainerC V>
V gaussianWeighting(const V& p, const V& t, double thres=0.6);
template <ContainerC V>
V gaussianClipping(const V& p, const V& t, double thres=0.6);

/// Filter the profile and date vectors by applying a predicate. The predicate takes as parameter the index of the vectors.
template <ContainerC V, DateContainerC W, PredicateC P>
std::pair<V, V> filter_profile(V vec, W date_vec, P pred);

/// Smooth a profile using a weighting of the type pow(fabs(t[i]-t[j]),alpha)
template <ContainerC V>
V smooth_profile(V profile, V t, unsigned int radius, double alpha);
/** Optimise the function f using a Levenberg Marquardt minimizer
    the x vector contains the initial guess and will hold the result.
    The function returns the optimisation error
*/
template<typename FunctionType>
double optimize(VectorType& x, ParameterCostFunction<FunctionType> f);

/// Types of fitting errors
enum class FittingErrors {OK, HasNan, NOK};

/// Filter a temporal profile y(t) with filter f and radius.
template <ContainerC V>
V profileFilter(const V& y, const V& t, typename std::function<double (V, const V&)> f, unsigned int radius);
/// Get the median of a vector
template <ContainerC V>
double getMedian(V y, const V&);
/// Get the value of the middle point of y(t) when approximated by a local parabola
template <ContainerC V>
double getLocalParabola(const V& y, const V& t);
/// Compute the 3 coefficients of a parabola which best approximates y(t)
template <ContainerC V>
V getParabola(const V& y, const V& t);

namespace normalized_sigmoid{
template <typename T, ContainerC V>
inline
T double_sigmoid(T t, const V& x);

/**
   The normalised double sigmoid or double logistic function is:
   f(t) = \frac{1}{1+exp(\frac{x_0-t}{x_1})}-\frac{1}{1+exp(\frac{x_2-t}{x_3})}
   Where:
   x_0 (resp. x_2) is the left (resp. right) inflection point
   x_1 (resp. x_3) is the rate of change at the left (resp. right) inflection point
*/
template <ContainerC V>
struct F
{
    inline void operator()(const V& t, const V& x, V& result) const
    {
      auto tsize(t.size());
      for(size_t i=0; i<tsize; ++i)
        result[i] = double_sigmoid(t[i], x);
    }
};

/// Estimates the first guess for the parameters given the profile p
template <ContainerC V>
V guesstimator(V p, V t);

/// Error diagnostics
template <ContainerC V>
FittingErrors error_diagnostics(const V& x, const V& p, const V& t);

/// Performs the approximation of the normalised sigmoid of profile(t)
template <ContainerC V>
ApproximationResultType Approximation(const V& profile, const V& t);
/** Performs the approximation of the normalised sigmoid of the principal cycle of profile(t) by
    1. applying the approximation once,
    2. applying the approximation to the residuals
    3. subtracting the approximated residuals from the original profile
    4. approximating the result of the subtraction
*/
template <ContainerC V, DateContainerC W>
ApproximationResultType PrincipalCycleApproximation(const V& profile, const W& t);

/** Performs the approximation of a profile as the sum of a pricipal cycle approximation and the
     approximation of the residuals. Returns the approximated profile and the results of the 2
     approximations.
*/
template <ContainerC V, DateContainerC W>
std::tuple<VectorType, ApproximationResultType, ApproximationResultType> TwoCycleApproximation(const V& profile, const W& t);

/** Return phenological metrics for the double sigmoid parameters as a tuple
    containing (max increasing slope, t0, t1, t2, t3, max decr. slope).
    t0 is the date for which the linear trend of the increasing slope crosses 0
    t3 is the t1 equivalent for the decreasing slope
    t1 is the date for which the linear trend of the incr. slope reaches the
       maximum of the double sigmoid
    t2 is the t1 equivalent for the decreasing slope
    the length of the plateau can be estimated by t2-t1
    The function can be used for non normalized double sigmoids by passing
    the minimum and the maximum values.                
*/
template <typename T, ContainerC V>
std::tuple<T, T, T, T, T, T>  pheno_metrics(const V& x, T maxvalue=1.0, 
                                            T minvalue=0.0);
}

/// The STICS function
namespace STICS{
VectorType F(const VectorType& t, const VectorType& x);
}

/// The double sigmoid
namespace sigmoid{
/**
   The double sigmoid or double logistic function is:
   f(t) = (\frac{1}{1+exp(\frac{x_0-t}{x_1})}-\frac{1}{1+exp(\frac{x_2-t}{x_3})})*x_4+x_5
   Where:
   x_0 (resp. x_2) is the left (resp. right) inflection point
   x_1 (resp. x_3) is the rate of change at the left (resp. right) inflection point
   x_4 is the amplitude (max-min)
   x_5 is the background (min) value
*/
template <ContainerC V>
void F(const V& t, const V& x, V& result);

/// Estimates the first guess for the parameters given the profile p
template <ContainerC V>
V guesstimator(V p, V t);

/// Estimates the first guess for the parameters given the profile p using a first Gaussian approximation
template <ContainerC V>
V guesstimator_with_gaussian(V p, V t);
}

/// Symmetric Gaussian
namespace gaussian{
/**
   The double gaussian  is:
   f(t) = (exp(-\frac{t-x0}{x_1})^2)*x_2+x_3
   Where:
   x_0 is the position of the maximum
   x_1 is the width
   x_2 is the amplitude (max-min)
   x_3 is the background (min) value
*/
VectorType F(const VectorType& t, const VectorType& x);

/// Estimates the first guess for the parameters given the profile p
VectorType guesstimator(VectorType p, VectorType t);
}

/// Convert from OTB pixel to vector
template<typename ValueType>
inline
VectorType pixelToVector(const itk::VariableLengthVector<ValueType>& p);

template <typename PixelType>
class TwoCycleSigmoFittingFunctor
{
protected:
  VectorType dv;
  bool return_fit;
  bool fit_only_invalid;
  bool use_mask;

public:
  struct DifferentSizes {};
  TwoCycleSigmoFittingFunctor() : return_fit{true}, fit_only_invalid{true},
                                  use_mask{true} {};

  void SetDates(const std::vector<tm>& d) {
    dv = VectorType{static_cast<unsigned int>(d.size())};
    const auto &days = pheno::tm_to_doy_list(d);
    std::copy(std::begin(days), std::end(days), std::begin(dv));
  }

  void SetUseMask(bool um) {
    use_mask = um;
  }

  void SetReturnFit(bool rf) {
    return_fit = rf;
  }

  void SetFitOnlyInvalid(bool foi) {
    fit_only_invalid = foi;
  }

  // dates is already given in doy
  // valid pixel has a mask==0
  PixelType operator()(PixelType pix, PixelType mask)
  {
    auto nbDates = pix.GetSize();

    auto tmp_mask = mask;
    if(!use_mask)
      tmp_mask.Fill(0);

    if(dv.size()!=nbDates) throw DifferentSizes{};

    PixelType tmpix{nbDates};
    tmpix.Fill(typename PixelType::ValueType{0});

    // If the mask says all dates are valid, keep the original value
    if(tmp_mask == tmpix && fit_only_invalid && use_mask) return pix;
    
    VectorType vec(nbDates);
    VectorType mv(nbDates);
    
    for(size_t i=0; i<nbDates; i++)
      {
      vec[i] = pix[i] / 10000;
      mv[i] = tmp_mask[i];
      }


    // A date is valid if it is not NaN and the mask value == 0.
    auto pred = [=](int e) { return !(std::isnan(vec[e])) && (vec[e] >= 0) &&
                             (mv[e]==(typename PixelType::ValueType{0})); };
    auto f_profiles = filter_profile_fast(vec, dv, pred);

    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;

    // If there are not enough valid dates, keep the original value
    if(profile.size() < 4) 
      {
      return pix;
      }

    auto approx = normalized_sigmoid::TwoCycleApproximation(profile, t);
    auto x_1 = std::get<0>(std::get<1>(approx));
    auto mm1 = std::get<1>(std::get<1>(approx));
    auto x_2 = std::get<0>(std::get<2>(approx));
    auto mm2 = std::get<1>(std::get<2>(approx));

    if(return_fit)
      {

      // The result uses either the original or the approximated value depending on the mask value
      PixelType result(nbDates);
      VectorType tmps1(nbDates);
      VectorType tmps2(nbDates);

      for(size_t i=0; i<nbDates; i++) {
          if(fabs(pix[i] - NO_DATA) < EPSILON)
              result[i] = NO_DATA;
          else {
              // Compute the approximation
              normalized_sigmoid::F<VectorType>()(dv, x_1, tmps1);
              normalized_sigmoid::F<VectorType>()(dv, x_2, tmps2);
              auto tmpres = tmps1*(mm1.second-mm1.first)+mm1.first
                + tmps2*(mm2.second-mm2.first)+mm2.first;
            if(fit_only_invalid)
              result[i] = ((mv[i]==(typename PixelType::ValueType{0}))?pix[i]:tmpres[i]);
            else
              result[i] = tmpres[i];
          }
      }

      return result;
      }
    else
      {
      PixelType result(12);
      result[0] = (mm1.second-mm1.first);
      result[1] = mm1.first;
      for(auto i=0; i<4; ++i)
        result[i+2] = x_1[i];

      result[6] = (mm2.second-mm2.first);
      result[7] = mm2.first;
      for(auto i=0; i<4; ++i)
        result[i+8] = x_2[i];

      return result;
      }
  }

  bool operator!=(const TwoCycleSigmoFittingFunctor a)
  {
    return (this->dates != a.dates) || (this->dv != a.dv) ;
  }

  bool operator==(const TwoCycleSigmoFittingFunctor a)
  {
    return !(*this == a);
  }
  
};
/** Binary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <class TInputImage ,class TOutputImage, class TFunctor>
class ITK_EXPORT BinaryFunctorImageFilterWithNBands : 
    public itk::BinaryFunctorImageFilter< TInputImage, TInputImage,
                                          TOutputImage, TFunctor >
{
public:
  typedef BinaryFunctorImageFilterWithNBands Self;
  typedef itk::BinaryFunctorImageFilter< TInputImage, TInputImage, 
                                         TOutputImage, 
                                         TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Macro defining the type*/
  itkTypeMacro(BinaryFunctorImageFilterWithNBands, SuperClass);
  
  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int);
  itkGetConstMacro(NumberOfOutputBands, unsigned int);
  
protected:
  BinaryFunctorImageFilterWithNBands() {}
  virtual ~BinaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  BinaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;


};
}
// include the definition of the template functions
#include "phenoFunctions.txx"
#endif
