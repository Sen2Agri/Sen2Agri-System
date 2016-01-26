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
#include <phenoFunctions.h>
#include "../../../../Composite/Common/GlobalDefs.h"

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

bool IsValidLandValue(float fValue, float fMskValue)
{
    if (fValue < NO_DATA_EPSILON) {
        return false;
    }
    if(fMskValue != IMG_FLG_LAND) {
        return false;
    }
    return true;
    //return fabs(fValue - NO_DATA_VALUE) < NO_DATA_EPSILON;
}

std::pair<VectorType, VectorType> 
fit_csdm(const VectorType &dts, const VectorType &ts, const VectorType &ets, const VectorType &msks)
{
  assert(ts.size()==ets.size() && ts.size()==dts.size() && ts.size()==msks.size());
  //auto result = ts;
  //auto result_flag = ts;
  auto result = VectorType(ts.size(), 0);
  auto result_flag = VectorType(ts.size(),0);
  // std::vector to vnl_vector
  pheno::VectorType profile_vec(ts.size());
  pheno::VectorType date_vec(dts.size());

  for(size_t i=0; i<ts.size(); i++)
    {
    profile_vec[i] = ts[i];
    date_vec[i] = dts[i];
    }

  // A date is valid if it is not NaN and the mask value == IMG_FLG_LAND.
  auto pred = [=](int e) { return !(std::isnan(profile_vec[e])) &&
                           IsValidLandValue(profile_vec[e], msks[e]); };
  auto f_profiles = pheno::filter_profile_fast(profile_vec, date_vec, pred);

  decltype(profile_vec) profile=f_profiles.first;
  decltype(profile_vec) t=f_profiles.second;
  if(profile.size() < 4)
  {
    return std::make_pair(result,result_flag);
  }

  // fit
  auto approximation_result = 
    pheno::normalized_sigmoid::TwoCycleApproximation(profile, t);
  auto princ_cycle = std::get<1>(approximation_result);
  auto x_hat = std::get<0>(princ_cycle);
  auto min_max = std::get<1>(princ_cycle);
  auto A_hat = min_max.second - min_max.first;
  auto B_hat = min_max.first;
  pheno::VectorType p(date_vec.size());
  pheno::normalized_sigmoid::F<pheno::VectorType>()(date_vec, x_hat, p);
  //fill the result vectors
  int validDatesCnt = 0;
  for(size_t i=0; i<ts.size(); i++)
  {
      result[i] = p[i]*A_hat+B_hat;
      if(result[i] < 0) {
          result[i] = 0;
      }
      if(IsValidLandValue(profile_vec[i], msks[i])) {
        result_flag[i] = ++validDatesCnt;
      } else {
        // use the last know processed value, if it exists
        result_flag[i] = validDatesCnt;
      }
  }

  return std::make_pair(result,result_flag);
}


std::pair<VectorType, VectorType> 
smooth_time_series_local_window_with_error(const VectorType &dts,
                                           const VectorType &ts,
                                           const VectorType &ets,
                                           const VectorType &msks,
                                           size_t bwd_radius = 1,
                                           size_t fwd_radius = 1)
{

    /**
        ------------------------------------
        |                    |             |
       win_first            current       win_last
    */
    assert(ts.size()==ets.size() && ts.size()==dts.size() && ts.size()==msks.size());
    auto result = ts;
    auto result_flag = VectorType(ts.size(),not_processed_value);
    auto ot = result.begin();
    auto otf = result_flag.begin();

    auto msk_first = msks.begin();

    auto eit = ets.begin();
    auto last = ts.end();
    auto win_first = ts.begin();
    auto win_last = ts.begin();
    auto e_win_first = ets.begin();
    auto e_win_last = ets.begin();
    auto dti = dts.begin();
    auto d_win_first = dts.begin();
    auto d_win_last = dts.begin();
    auto win_msk_first = msks.begin();
    //advance iterators
    std::advance(ot, bwd_radius);
    std::advance(otf, bwd_radius);
    std::advance(eit, bwd_radius);
    std::advance(dti, bwd_radius);
    std::advance(win_msk_first, bwd_radius);
    std::advance(win_last, bwd_radius+fwd_radius);
    std::advance(e_win_last, bwd_radius+fwd_radius);
    std::advance(d_win_last, bwd_radius+fwd_radius);

    // vector having the bwr size containing the last valid (land) values
    VectorType lastValidValues(bwd_radius);
    VectorType lastValidDates(bwd_radius);
    VectorType lastValidErrs(bwd_radius);
    size_t lastValidValuesCnt = 0;
    for(size_t i = 0; i < bwd_radius; i++) {
        // add the valid values, if any, from 0 to bwd
        if(msks[i] == IMG_FLG_LAND) {
            lastValidValues[lastValidValuesCnt] = ts[i];
            lastValidDates[lastValidValuesCnt] = dts[i];
            lastValidErrs[lastValidValuesCnt] = ets[i];
            lastValidValuesCnt++;
        }
    }

    while(win_last!=last)
    {
        auto current_d = d_win_first;
        auto current_e = e_win_first;
        auto current_v = win_first;
        auto current_msk = win_msk_first;
        auto past_it = d_win_last; ++past_it;

        PrecisionType sum_weights{0.0};
        PrecisionType weighted_value{0.0};
        size_t nProcessedVals = 0;
        while(current_d != past_it)
        {
            // If the mask flag is LAND, we process the value
            if(*current_msk == IMG_FLG_LAND) {
                auto cw = compute_weight(fabs(*current_d-*dti),fabs(*current_e));
                sum_weights += cw;
                weighted_value += (*current_v)*cw;
                ++nProcessedVals;
            }
            ++current_d;
            ++current_e;
            ++current_v;
            ++current_msk;
        }

        if(nProcessedVals >= bwd_radius) {
            *ot = weighted_value/sum_weights;
            *otf = nProcessedVals;
        } else {
            // we try to use the last valid values buffer
            if ((nProcessedVals + lastValidValuesCnt) >= bwd_radius) {
                // in this case we are able to compute a valid value
                int remVals = bwd_radius - nProcessedVals;
                for(int i = 0; i<remVals; i++) {
                    // we take the values from the last valid one towards the beginning
                    int idx = lastValidValuesCnt - i - 1;
                    auto cw = compute_weight(fabs(lastValidDates[idx]-*dti),fabs(lastValidErrs[idx]));
                    sum_weights += cw;
                    weighted_value += (lastValidValues[idx])*cw;
                    ++nProcessedVals;
                }

                //recompute the LAI and the flags
                *ot = weighted_value/sum_weights;
                *otf = nProcessedVals;
            }
            // otherwise, the value remains the same and the flag is not processed
        }
        // update the last valid values buffer if it is the case
        if(*win_msk_first == IMG_FLG_LAND) {
            if(lastValidValuesCnt == bwd_radius) {
                //remove the first element in the vector
                lastValidValues.erase(lastValidValues.begin());
                lastValidDates.erase(lastValidDates.begin());
                lastValidErrs.erase(lastValidErrs.begin());

                lastValidValues.push_back(*win_first);
                lastValidDates.push_back(*d_win_first);
                lastValidErrs.push_back(*e_win_first);
            } else {
                lastValidValues[lastValidValuesCnt] = *win_first;
                lastValidDates[lastValidValuesCnt] = *d_win_first;
                lastValidErrs[lastValidValuesCnt] = *e_win_first;
                lastValidValuesCnt++;
            }
        }
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
        ++msk_first;
        ++win_msk_first;
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
