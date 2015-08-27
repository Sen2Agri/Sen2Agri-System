/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __OTBBVPROFREPR_H
#define __OTBBVPROFREPR_H

#include <vector>

using PrecisionType = double;
using VectorType = std::vector<PrecisionType>;
constexpr PrecisionType not_processed_value{0};
constexpr PrecisionType processed_value{1};


namespace otb
{
template <typename T>
inline 
T compute_weight(T delta, T err)
{
  T one{1};
  return (one/(one+delta)+one/(one+err));
}

std::pair<VectorType, VectorType> 
fit_csdm(VectorType dts, VectorType ts, VectorType ets)
{
  assert(ts.size()==ets.size() && ts.size()==dts.size());
  auto result = ts;
  auto result_flag = ts;
  // std::vector to vnl_vector
  pheno::VectorType profile_vec(ts.size());
  pheno::VectorType date_vec(dts.size());

  for(size_t i=0; i<ts.size(); i++)
    {
    profile_vec[i] = ts[i];
    date_vec[i] = dts[i];
    }

  // fit
  auto approximation_result = 
    pheno::normalized_sigmoid::TwoCycleApproximation(profile_vec, date_vec);
  auto princ_cycle = std::get<1>(approximation_result);
  auto x_hat = std::get<0>(princ_cycle);
  auto min_max = std::get<1>(princ_cycle);
  auto A_hat = min_max.second - min_max.first;
  auto B_hat = min_max.first;
  auto p = pheno::normalized_sigmoid::F(date_vec, x_hat);
  //fill the result vectors
  for(size_t i=0; i<ts.size(); i++)
    {
    result[i] = p[i]*A_hat+B_hat;
    result_flag[i] = processed_value;
    }

  return std::make_pair(result,result_flag);
}


std::pair<VectorType, VectorType> 
smooth_time_series_local_window_with_error(VectorType dts,
                                           VectorType ts, 
                                           VectorType ets,
                                           size_t bwd_radius = 1,
                                           size_t fwd_radius = 1)
{

  /**
        ------------------------------------
        |                    |             |
       win_first            current       win_last
  */
  assert(ts.size()==ets.size() && ts.size()==dts.size());
  auto result = ts;
  auto result_flag = VectorType(ts.size(),not_processed_value);
  auto ot = result.begin();
  auto otf = result_flag.begin();
  auto eit = ets.begin();
  auto last = ts.end();
  auto win_first = ts.begin();
  auto win_last = ts.begin();
  auto e_win_first = ets.begin();
  auto e_win_last = ets.begin();
  auto dti = dts.begin();
  auto d_win_first = dts.begin();
  auto d_win_last = dts.begin();
//advance iterators
std::advance(ot, bwd_radius);
std::advance(otf, bwd_radius);
std::advance(eit, bwd_radius);
std::advance(dti, bwd_radius);
std::advance(win_last, bwd_radius+fwd_radius);
std::advance(e_win_last, bwd_radius+fwd_radius);
std::advance(d_win_last, bwd_radius+fwd_radius);
while(win_last!=last)
  {
  auto current_d = d_win_first;
  auto current_e = e_win_first;
  auto current_v = win_first;
  auto past_it = d_win_last; ++past_it;

  PrecisionType sum_weights{0.0};
  PrecisionType weighted_value{0.0};
  while(current_d != past_it)
    {
    auto cw = compute_weight(fabs(*current_d-*dti),fabs(*current_e));
    sum_weights += cw;
    weighted_value += (*current_v)*cw;
    ++current_d;
    ++current_e;
    ++current_v;
    }
  *ot = weighted_value/sum_weights;
  *otf = processed_value;
  ++win_first;
  ++win_last;
  ++e_win_first;
  ++e_win_last;
  ++d_win_first;
  ++d_win_last;
  ++ot;
  ++otf;
  ++eit;
  ++dti;
  }
return std::make_pair(result,result_flag);
}

VectorType smooth_time_series(VectorType ts, PrecisionType alpha, 
                              bool online=true)
{
  auto result = ts;
  auto it = ts.begin();
  auto ot = result.begin();
  auto last = ts.end();
  auto prev = *it;
  while(it!=last)
    {
    *ot = (*it)*(1-alpha)+alpha*prev;
    if(online)
      prev = *ot;
    else
      prev = *it;
    ++it;
    ++ot;
    }
  return result;
}

//assumes regular time sampling
VectorType smooth_time_series_n_minus_1(VectorType ts, PrecisionType alpha)
{
  auto result = ts;
  auto ot = result.begin();
  auto last = ts.end();
  auto prev = ts.begin();
  auto next = ts.begin();
  //advance iterators
  ++ot;
  ++next;
  while(next!=last)
    {
    auto lin_interp = ((*prev)+(*next))/2.0;
    *ot = (lin_interp)*(1-alpha)+alpha*(*ot);
    ++prev;
    ++next;
    ++ot;
    }
  return result;
}

}

#endif
