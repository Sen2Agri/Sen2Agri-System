/*=========================================================================

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <cassert>

namespace pheno
{
template <ContainerC V, DateContainerC W, PredicateC P>
inline
std::pair<V, V> filter_profile_fast(V vec, W date_vec, P pred)
{
  if(vec.size()!=date_vec.size())
    itkGenericExceptionMacro(<< "Profile vector and date vector have different sizes: " << vec.size() << "/" << date_vec.size()<< "\n");

  size_t sz = vec.size();
  size_t nbDates = 0;
  for(size_t i=0; i<sz; i++) if(pred(i)) nbDates++;
  V profile(nbDates);
  V t(nbDates);
  size_t dc=0;
  for (size_t i=0; i<sz; i++)
    if (pred(i))
      {
        profile[dc] = vec[i];
        t[dc] = date_vec[i];
        dc++;
      }
  return {profile,t};
}

template <ContainerC V, DateContainerC W, PredicateC P>
inline
std::pair<V, V> filter_profile(V vec, W date_vec, P pred)
{
  if(vec.size()!=date_vec.size())
    itkGenericExceptionMacro(<< "Profile vector and date vector have different sizes: " << vec.size() << "/" << date_vec.size()<< "\n");
    
  unsigned int nbDates{0};
  for(size_t i=0; i<vec.size(); i++) if(pred(i)) nbDates++;
  V profile(nbDates);
  V t(nbDates);
  int dc{0};
  for(size_t i=0; i<vec.size(); i++)
    if(pred(i))
      {
      profile[dc]=vec[i];
      t[dc]=doy(date_vec[i]);
      dc++;
      }
  unsigned int year_count = 0;
  for(size_t i=1; i<nbDates; ++i)
    {
    //std::cout << t[i] << " ";
    auto tmp_doy = t[i];
    if(tmp_doy < t[i-1]-365*year_count)
      ++year_count;
    t[i] += 365*year_count;
    }

  return {profile,t};
}

template <ContainerC V>
inline
V smooth_profile(V profile, V t, unsigned int radius, double alpha) {
  assert(profile.size() == t.size());
  assert(radius < t.size()/2);
  V res(profile.size());

  for(auto i=0; i<profile.size(); ++i)
    {
    auto first_val = (i<radius)?0:(i-radius);
    auto last_val = (i>(t.size()-radius-1))?(t.size()-1):(i+radius);
    double numer{0};
    double denom{0};
    for(auto j=first_val; j<=last_val; ++j)
      {
      double weight = (i==j)?1.0:pow(fabs(t[i]-t[j]),alpha);
      numer += profile[j]/weight;
      denom += 1.0/weight;
      }
    res[i] = numer/denom;
    }

  return res;
}

template<typename FunctionType>
double optimize(VectorType& x, ParameterCostFunction<FunctionType> f)
{
// Make a Levenberg Marquardt minimizer, attach f to it, and
// run the minimization
  vnl_levenberg_marquardt levmarq(f);
  levmarq.set_f_tolerance(1e-10);
  levmarq.set_x_tolerance(1e-100);
  levmarq.set_g_tolerance(1e-100);
  levmarq.minimize(x);
  return levmarq.get_end_error();
}

template<typename ValueType>
inline
VectorType pixelToVector(const itk::VariableLengthVector<ValueType>& p)
{
  VectorType v(p.Size());
  for(size_t i=0; i<p.Size(); ++i)
    v[i] = ValueType{p[i]};
  return v;
}

template<typename ValueType>
inline
itk::VariableLengthVector<ValueType> vectorToPixel(const VectorType& v)
{
  itk::VariableLengthVector<ValueType> p(v.size());
  for(auto i=0; i<p.Size(); ++i)
    p[i] = ValueType{v[i]};
  return p;
}

template<typename ValueType>
inline
itk::VariableLengthVector<ValueType> vectorToPixel(const std::vector<ValueType>& v)
{
  itk::VariableLengthVector<ValueType> p(v.size());
  for(size_t i=0; i<p.Size(); ++i)
    p[i] = ValueType{v[i]};
  return p;
}

namespace sigmoid{
template <ContainerC V>
inline
void F(const V& t, const V& x, V& result)
{
  auto tsize = t.size();
  for(size_t i=0; i<tsize; ++i)
    result[i] = x[4]*(1.0/(1.0+exp((x[0]-t[i])/x[1]))-1.0/(1.0+exp((x[2]-t[i])/x[3])))+x[5];
}
template <ContainerC V>
inline
V guesstimator(V p, V t)
{
  // Find the max and min elements
  auto minmax = std::minmax_element(std::begin(p), std::end(p));
  auto vmax = *(minmax.second);
  auto vmin = *(minmax.first);
  auto pmax = minmax.second - std::begin(p);
  V x0(6);
  x0[0] = t[pmax] - 50;
  x0[1] = 10.0;
  x0[2] = t[pmax] + 50;
  x0[3] = 10.0;
  x0[4] = (vmax-vmin);
  x0[5] = vmin;
  return x0;
}

template <ContainerC V>
inline
V guesstimator_with_gaussian(V p, V t)
{
  V xg(gaussian::guesstimator(p, t));
  auto phF(gaussian::F);
  ParameterCostFunction<decltype(phF)> f{4, t.size(), p, t, phF};
  optimize(xg, f);
  V xs(6);
  xs[0] = xg[0] - 2*xg[1]; // maxpos - stdev
  xs[1] = 2*xg[2]/xg[1]*(exp(-vnl_math_sqr((xs[0]-xg[0])/xg[1])));
  xs[2] = xg[0] + 2*xg[1]; // maxpos + stdev
  xs[3] = -2*xg[2]/xg[1]*(exp(-vnl_math_sqr((xs[2]-xg[0])/xg[1])));
  xs[4] = xg[2];
  xs[5] = xg[3];
  return xs;
}
}

// normalized_sigmoid
namespace normalized_sigmoid{
template <typename T, ContainerC V>
inline
T double_sigmoid(T t, const V& x)
{
  return 1.0/(1.0+exp((x[0]-t)/x[1]))-1.0/(1.0+exp((x[2]-t)/x[3]));
}

template <typename T>
inline
T diff_sigmoid(T t, T x0, T x1)
{
  auto b = exp((x0-t)/x1);
  return 1.0/(x1*(1+1/b)*(1+b));
}

template <typename T, ContainerC V>
inline
T diff_double_sigmoid(T t, const V& x)
{
  return diff_sigmoid(t, x[0], x[1])-diff_sigmoid(t, x[2], x[3]);
}

template <typename T, ContainerC V>
inline
std::tuple<T, T, T, T, T, T> pheno_metrics(const V& x, T maxvalue,
                                           T minvalue)
{
  auto A = maxvalue-minvalue;
  auto B = minvalue;
  auto gx0 = A*double_sigmoid(x[0], x)+B;
  auto dgx0 = A*diff_double_sigmoid(x[0], x);
  auto gx2 = A*double_sigmoid(x[2], x)+B;
  auto dgx2 = A*diff_double_sigmoid(x[2], x);
  
  auto t0 = x[0] - gx0/dgx0;
  auto t1 = (A+B-(gx0-dgx0*x[0]))/dgx0;
  auto t2 = (A+B-(gx2-dgx2*x[2]))/dgx2;
  auto t3 = x[2] - gx2/dgx2;

  return std::make_tuple(dgx0, t0, t1, t2, t3, dgx2);
}



template <ContainerC V>
inline
V guesstimator(V p, V t)
{
  // Find the max position
  auto pmax = std::max_element(std::begin(p), std::end(p)) - std::begin(p);  
  V x0(4);
  x0[0] = t[pmax] - 25;
  x0[1] = 10.0;
  x0[2] = t[pmax] + 25;
  x0[3] = 10.0;
  return x0;
}

template <ContainerC V>
inline
FittingErrors error_diagnostics(const V& x, const V& p, const V& t)
{
  if(std::any_of(std::begin(x), std::end(x), [](double v){return std::isnan(v);}))
    return FittingErrors::HasNan;
  auto pmax = (std::minmax_element(std::begin(p), std::end(p))).second - std::begin(p);

  if(x[0] < -50 ||
     x[0] > t[pmax] ||
     x[2] < t[pmax] ||
     x[2] > 400 ||
     x[1] < 0 ||
     x[1] > 60 ||
     x[3] < 0 ||
     x[3] > 60 ||
     pmax ==0 ||
     pmax == t.size()-1)
    return FittingErrors::NOK;

  return FittingErrors::OK;

}

template <ContainerC V>
ApproximationResultType Approximation(const V& profile, const V& t)
{
  auto minmax = std::minmax_element(std::begin(profile), std::end(profile));
  auto t_max = std::begin(t);
  std::advance(t_max,std::distance(std::begin(profile),minmax.second));
  auto vmax = *(minmax.second);
  auto vmin = *(minmax.first);
  auto prof = (profile-vmin)/(vmax-vmin);
  auto x(guesstimator(prof, t));
  auto fprofile = gaussianWeighting(prof, t, *t_max,75.0);
  ParameterCostFunction<F<V>> fs{4, t.size(), fprofile, t, F<V>() };
  auto err(optimize(x, fs));
  V yHat(t.size());
  F<V>()(t,x,yHat);
  auto residuals = prof - yHat;
  yHat = yHat*(vmax-vmin)+vmin;
  residuals = residuals*(vmax-vmin);
  auto mm = std::make_pair(vmin, vmax);
  return ApproximationResultType{x, mm, yHat, residuals, err};
}

template <ContainerC V, DateContainerC W>
ApproximationResultType PrincipalCycleApproximation(const V& profile, const W& t)
{
  // Profile approximation
  auto approx_result = Approximation(profile, t);
  auto mm = std::get<1>(approx_result);
  auto residuals = std::get<3>(approx_result)+mm.first;

  // Residual approximation
  approx_result = Approximation(residuals, t);


  // Subtract the residual approximation from the original profile
  auto yHat = std::get<2>(approx_result);
  residuals = profile-yHat;

  // Approx the original minus the residuals
  approx_result = Approximation(residuals, t);

  // Get the data to be returned
  auto x = std::get<0>(approx_result);
  auto minmax = std::get<1>(approx_result);
  yHat = std::get<2>(approx_result);
  residuals = profile-yHat;
  auto err = std::get<4>(approx_result);

  return ApproximationResultType{x, minmax, yHat, residuals, err};
}

template <ContainerC V, DateContainerC W>
std::tuple<VectorType, ApproximationResultType, ApproximationResultType> TwoCycleApproximation(const V& profile, const W& t)
{
  auto pca = PrincipalCycleApproximation(profile, t);
  // Approximate the residuals
  auto residual_approx = Approximation(std::get<3>(pca), t);
  // The approximated profile is the sum of the approximations
  auto yHat = std::get<2>(pca)+std::get<2>(residual_approx);
  // Return {x, minmax, yHat, residuals, err};
  return std::make_tuple(yHat, pca, residual_approx);
}
}

template <ContainerC V>
inline
V gaussianWeighting(const V& p, const V& t, double m, double s)
{
  auto fp = p;
  for(size_t i=0; i<t.size(); i++)
    {
    fp[i] *= gaussianFunction(t[i],m,s);
    }

  return fp;
}

template <ContainerC V>
inline
V gaussianWeighting(const V& p, const V& t, double thres)
{
  auto pmax = std::minmax_element(std::begin(p), std::end(p)).second - std::begin(p);
  double threshold = p[pmax] * thres;
  auto pthres =  pmax;
  while((pthres>=0) && p[pthres]>threshold) --pthres;
  auto pbegin = ++pthres;
  pthres = pmax;
  while((pthres<p.size()) && p[pthres]>threshold) ++pthres;
  auto pend = --pthres;

  auto tbegin = t[pbegin];
  auto tend = t[pend];

  auto fp = p;
  for(size_t i=0; i<t.size(); i++)
    {
    fp[i] *= gaussianFunction(t[i],t[pmax],(tend-tbegin));
    }

  return fp;
}

template <ContainerC V>
inline
V gaussianClipping(const V& p, const V& t, double thres)
{
  auto pmax = std::minmax_element(std::begin(p), std::end(p)).second - std::begin(p);
  double threshold = p[pmax] * thres;
  auto pthres =  pmax-1;
  while((pthres>=0)
        && p[pthres]>threshold
        && (p[pthres]<=p[pthres+1])
    )
    {
    --pthres;
    }
  auto pbegin = ++pthres;
  pthres = pmax+1;
  while((pthres<p.size())
        && p[pthres]>threshold
        && (p[pthres]<=p[pthres-1])
    )
    {
    ++pthres;
    }
  auto pend = --pthres;

  auto tbegin = t[pbegin];
  auto tend = t[pend];

  double decay{0.1};
  
  auto fp = p;
  for(auto i=0; i<t.size(); i++)
    {
    if(i<pbegin || i>pend)
      fp[i] *= gaussianFunction(t[i],t[pmax],decay);
    }

  return fp;
}

template <ContainerC V>
inline
V profileFilter(const V& y, const V& t, typename std::function<double (V, const V&)> f, unsigned int radius)
{
  const size_t nbDates = y.size();
  assert(nbDates >= radius);

  V res(nbDates);

  for(size_t i=0; i<nbDates; i++)
    {
    if(i>radius && i<static_cast<size_t>(nbDates-radius))
      {
      V tmpp(2*radius+1);
      V tmpd(2*radius+1);
      for(size_t k=0; k<=2*radius; k++)
        {
        tmpp[k] = y[i-radius+k];
        tmpd[k] = t[i-radius+k];
        }
      res[i] = f(tmpp, tmpd);
      }
    else res[i] = y[i];
    }
  return res;
}

template <ContainerC V>
inline
V getParabola(const V& y, const V& t)
{
  const unsigned int nbDates = y.size();
  const unsigned int nbCoefs = 3;
  // b = A * c
  vnl_matrix<double> A(nbDates, nbCoefs);
  vnl_matrix<double> b(nbDates, 1);

  // fill the matrices
  for(size_t i = 0; i < nbDates; ++i)
    {
    b.put(i, 0, y[i]);
    A.put(i, 0, t[i]*t[i]);
    A.put(i, 1, t[i]);
    A.put(i, 2, 1.0);
    }
  // solve the problem c = (At * A)^-1*At*b

  vnl_matrix<double> atainv =  vnl_matrix_inverse<double>(vnl_transpose(A) * A);

  vnl_matrix<double> atainvat = atainv * vnl_transpose(A);
  vnl_matrix<double> c = atainvat * b;

  V res(nbCoefs);

  for(size_t i=0; i<nbCoefs; i++) res[i] = c.get(i, 0);

  return res;
}

template <ContainerC V>
inline
double getMedian(V y, const V&)
{
  std::sort(std::begin(y), std::end(y));
  return y[y.size()/2];
}

template <ContainerC V>
inline
double getLocalParabola(const V& y, const V& t)
{
  auto par = getParabola(y, t);
  auto a0hat = -par[0];
  auto t0hat = par[1]/(2*a0hat);
  auto v0hat = par[2]+a0hat*vnl_math_sqr(t0hat);
  auto tcenter = t[(t.size()-1)/2];
  return v0hat-a0hat*vnl_math_sqr(tcenter-t0hat);
}

}
