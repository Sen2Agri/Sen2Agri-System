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

#include <vector>

#include "dateUtils.h"
#include "phenoFunctions.h"
#include "otbProfileReprocessing.h"

namespace otb
{
int date_to_doy(std::string& date_str)
{
  return pheno::doy(pheno::make_date(date_str));
}


namespace Wrapper
{

class ProfileReprocessing : public Application
{
public:
  /** Standard class typedefs. */
  typedef ProfileReprocessing               Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(ProfileReprocessing, otb::Application);

private:
  void DoInit()
  {

    SetName("ProfileReprocessing");
    SetDescription("Reprocess a BV time profile.");
   
    AddParameter(ParameterType_InputFilename, "ipf", "Input profile file.");
    SetParameterDescription( "ipf", "Input file containing the profile to process. This is an ASCII file where each line contains the date (YYYMMDD) the BV estimation and the error." );
    MandatoryOn("ipf");

    AddParameter(ParameterType_OutputFilename, "opf", "Output profile file.");
    SetParameterDescription( "opf", "Filename where the reprocessed profile saved. This is an ASCII file where each line contains the date (YYYMMDD) the new BV estimation and a boolean information which is 0 if the value has not been reprocessed." );
    MandatoryOn("opf");

    AddParameter(ParameterType_Choice, "algo", 
                 "Reprocessing algorithm: local, fit.");
    SetParameterDescription("algo", 
                            "Reprocessing algorithm: local uses a window around the current date, fit is a double logisting fitting of the complete profile.");

    AddChoice("algo.fit", "Double logistic fitting of the complete profile.");
    SetParameterDescription("algo.fit", "This group of parameters allows to set fit window parameters. ");

    AddChoice("algo.local", "Uses a window around the current date.");
    SetParameterDescription("algo.local", "This group of parameters allows to set local window parameters. ");

    AddParameter(ParameterType_Int, "algo.local.bwr", "Local window backward radius");
    SetParameterInt("algo.local.bwr", 1);
    SetParameterDescription("algo.local.bwr", "Backward radius of the local window. ");

    AddParameter(ParameterType_Int, "algo.local.fwr", "Local window forward radius");
    SetParameterInt("algo.local.fwr", 1);
    SetParameterDescription("algo.local.fwr", "Forward radius of the local window. ");

    MandatoryOff("algo");
  }

  void DoUpdateParameters()
  {
    //std::cout << "ProfileReprocessing::DoUpdateParameters" << std::endl;
  }

  void DoExecute()
  {
    auto ipfn = GetParameterString("ipf");
    std::ifstream in_profile_file;
    try
      {
      in_profile_file.open(ipfn.c_str());
      }
    catch(...)
      {
      itkGenericExceptionMacro(<< "Could not open file " 
                               << ipfn);
      }

    auto nb_dates = 0;
    std::vector<std::string> date_str_vec{};
    VectorType date_vec{};
    VectorType bv_vec{};
    VectorType err_vec{};
    for(std::string line; std::getline(in_profile_file, line); )
      {
      if(line.size() > 1)
        {
        std::istringstream ss(line);
        std::string date_str;
        PrecisionType in_bv;
        PrecisionType bv_err;

        ss >> date_str;
        ss >> in_bv;
        ss >> bv_err;

        date_str_vec.push_back(date_str);
        date_vec.push_back(date_to_doy(date_str));
        bv_vec.push_back(in_bv);
        err_vec.push_back(bv_err);

        nb_dates++;
        }
      }
    otbAppLogINFO("Input profile contains " << nb_dates << " dates.\n");

    in_profile_file.close();

    VectorType out_bv_vec{};
    VectorType out_flag_vec{};

    std::string algo{"local"};
    if (IsParameterEnabled("algo"))
      algo = GetParameterString("algo");    
    if (algo == "local")
      {
      size_t bwr{1};
      size_t fwr{1};
      if (IsParameterEnabled("algo.local.bwr"))
        bwr = GetParameterInt("algo.local.bwr");
      if (IsParameterEnabled("algo.local.fwr"))
        bwr = GetParameterInt("algo.local.fwr");
      std::tie(out_bv_vec, out_flag_vec) = 
        smooth_time_series_local_window_with_error(date_vec, bv_vec, err_vec, 
                                                   bwr, fwr);
      }
    else if (algo == "fit")
      std::tie(out_bv_vec, out_flag_vec) = 
        fit_csdm(date_vec, bv_vec, err_vec);
    else
      itkGenericExceptionMacro(<< "Unknown algorithm " << algo 
                               << ". Available algorithms are: local, fit.\n");

    auto opfn = GetParameterString("opf");
    std::ofstream out_profile_file;
    try
      {
      out_profile_file.open(opfn.c_str());
      }
    catch(...)
      {
      itkGenericExceptionMacro(<< "Could not open file " << opfn);
      }
    for(size_t i=0; i<date_vec.size(); ++i)
      {
      std::stringstream ss;
      ss << date_str_vec[i] << "\t" << out_bv_vec[i] << "\t" 
         << out_flag_vec[i] << std::endl;
      out_profile_file << ss.str();
      }

    out_profile_file.close();


  }
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProfileReprocessing)
