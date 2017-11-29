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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperChoiceParameter.h"

#include <random>
#include <fstream>

#include "otbBVTypes.h"

namespace otb
{

namespace Wrapper
{

class BVInputVariableGeneration : public Application
{
public:
/** Standard class typedefs. */
  typedef BVInputVariableGeneration     Self;
  typedef Application                   Superclass;

  enum DistType {GAUSSIAN, UNIFORM, LOGNORMAL};
  
/** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(BVInputVariableGeneration, otb::Application);

  typedef std::map< IVNames, double > SampleType;
private:
  void DoInit()
  {
    SetName("BVInputVariableGeneration");
    SetDescription("Generate random input variable distribution for ... .");

    AddDocTag(Tags::BV);
    AddParameter(ParameterType_Int, "samples", "Sample size");
    SetDefaultParameterInt("samples", 1000);
    SetParameterDescription("samples", "Number of samples to be generated");
    
    AddParameter(ParameterType_OutputFilename, "out", "Output file");
    SetParameterDescription( "out", "Filename where the variable sets are saved." );
    MandatoryOn("out");

    AddParameter(ParameterType_Float, "minlai", "Minimum value for LAI");
    SetDefaultParameterFloat("minlai", 0.0);
    SetParameterDescription("minlai", "Minimum value for LAI");

    AddParameter(ParameterType_Float, "maxlai", "Maximum value for LAI");
    SetDefaultParameterFloat("maxlai", 15.0);
    SetParameterDescription("maxlai", "Maximum value for LAI");

    AddParameter(ParameterType_Float, "modlai", "Mode value for LAI");
    SetDefaultParameterFloat("modlai", 2.0);
    SetParameterDescription("modlai", "Mode value for LAI");

    AddParameter(ParameterType_Float, "stdlai", "Standard deviation value for LAI");
    SetDefaultParameterFloat("stdlai", 2.0);
    SetParameterDescription("stdlai", "Standard deviation value for LAI");

    AddParameter(ParameterType_Float, "minala", "Minimum value for ALA");
    SetDefaultParameterFloat("minala", 30.0);
    SetParameterDescription("minala", "Minimum value for ALA");

    AddParameter(ParameterType_Float, "maxala", "Maximum value for ALA");
    SetDefaultParameterFloat("maxala", 80.0);
    SetParameterDescription("maxala", "Maximum value for ALA");

    AddParameter(ParameterType_Float, "modala", "Mode value for ALA");
    SetDefaultParameterFloat("modala", 60.0);
    SetParameterDescription("modala", "Mode value for ALA");

    AddParameter(ParameterType_Float, "stdala", "Standard deviation value for ALA");
    SetDefaultParameterFloat("stdala", 20.0);
    SetParameterDescription("stdala", "Standard deviation value for ALA");

  }

  virtual ~BVInputVariableGeneration()
  {
  }


  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  //Generates a random number of the appropriate distribution and respecting the bounds
  double Rng(double min, double max, double mod, double stdev, DistType dist)
  {
    //TODO : why us stdev defined for uniform?
    double rn;
    if(dist == GAUSSIAN)
      {
      auto sampleInsideBounds = false;
      while(!sampleInsideBounds)
        {
        std::normal_distribution<> d(mod,stdev);
        rn = d(m_RNG);
        if( rn >= min && rn <= max)
          sampleInsideBounds = true;
        }
      }
    if(dist == LOGNORMAL)
      {
      auto sampleInsideBounds = false;
      while(!sampleInsideBounds)
        {
        std::lognormal_distribution<> d(mod, stdev);
        rn = d(m_RNG);
        if( rn >= min && rn <= max)
          sampleInsideBounds = true;
        }
      }
    else
      {
      std::uniform_real_distribution<> d(min, max);
      rn = d(m_RNG);
      }




    return rn;
  }


  /**
     The LAI_Conv parameter is used to link (cross-correlation) each
     variable to the LAI for a given sample using:

                 V* = V_mod + (V -V_mod)*(LAI_Conv - MLAI)/LAI_Conv

     where V* is the value of variable V after the correlation.
  */
  double CorrelateValue(double v, double v_mod, double lai)
  {
    double res = v_mod + (v - v_mod)*(m_LAI_Conv - lai)/m_LAI_Conv;
    return res<0?0:res;
  }
  
  ///Builds the map with the values of the sample
  SampleType DrawSample()
  {
    SampleType s;
    s[IVNames::MLAI] = this->Rng(m_MLAI_min, m_MLAI_max, m_MLAI_mod, m_MLAI_std, LOGNORMAL);
    s[IVNames::ALA] = this->CorrelateValue(this->Rng(m_ALA_min, m_ALA_max, m_ALA_mod,
                                            m_ALA_std, GAUSSIAN),
                                  m_ALA_mod, s[IVNames::MLAI]);
    s[IVNames::CrownCover] = this->CorrelateValue(this->Rng(m_CrownCover_min, m_CrownCover_max,
                                                   m_CrownCover_mod, m_CrownCover_std, UNIFORM),
                                         m_CrownCover_mod, s[IVNames::MLAI]);
    s[IVNames::HsD] = this->Rng(m_HsD_min, m_HsD_max, m_HsD_mod, m_HsD_std, GAUSSIAN);
    s[IVNames::N] = this->CorrelateValue(this->Rng(m_N_min, m_N_max, m_N_mod, m_N_std, GAUSSIAN),
                                m_N_mod, s[IVNames::MLAI]);
    s[IVNames::Cab] = this->CorrelateValue(this->Rng(m_Cab_min, m_Cab_max, m_Cab_mod,
                                            m_Cab_std, GAUSSIAN),
                                  m_Cab_mod, s[IVNames::MLAI]);
    s[IVNames::Car] = s[IVNames::Cab]*0.25;
    s[IVNames::Cdm] = this->CorrelateValue(this->Rng(m_Cdm_min, m_Cdm_max, m_Cdm_mod,
                                            m_Cdm_std, GAUSSIAN),
                                  m_Cdm_mod, s[IVNames::MLAI]);
    s[IVNames::CwRel] = this->CorrelateValue(this->Rng(m_CwRel_min, m_CwRel_max, m_CwRel_mod,
                                              m_CwRel_std, UNIFORM),
                                    m_CwRel_mod, s[IVNames::MLAI]);
    s[IVNames::Cbp] = this->CorrelateValue(this->Rng(m_Cbp_min, m_Cbp_max, m_Cbp_mod,
                                            m_Cbp_std, GAUSSIAN),
                                  m_Cbp_mod, s[IVNames::MLAI]);
    s[IVNames::Bs] = this->CorrelateValue(this->Rng(m_Bs_min, m_Bs_max, m_Bs_mod, m_Bs_std, GAUSSIAN),
                                 m_Bs_mod, s[IVNames::MLAI]);

    return s;
  }

  void WriteSample(SampleType s)
  {
    auto si = s.begin();
    while( si != s.end())
      {
      m_SampleFile << std::setw(12) << std::left << (*si).second ;
      ++si;
      }
    m_SampleFile << std::endl;
  }

  void DoExecute()
  {
    /*
     The LAI_Conv parameter is used to link (cross-correlation) each
     variable to the LAI for a given sample using:

                 V* = V_mod + (V -V_mod)*(LAI_Conv - MLAI)/LAI_Conv

     where V* is the value of variable V after the correlation.

     High values of LAI_Conv mean no correlation. Since, except for
     HsDn which is not correlated, all other values are equal to 10,
     we will only define one variable to deal with that, instead of
     defining a value for each bv.

     Car --> not used by bvnet; Féret's dissertation uses Cxc to denote Car
     and gives a mean of 8.58 and a stdev of 3.95 and fig 2.2, p.47 gives the
     min at 0 and the max at 25
     
  */

    m_MLAI_min = GetParameterFloat("minlai");
    m_MLAI_max = GetParameterFloat("maxlai");
    m_MLAI_mod = GetParameterFloat("modlai");
    m_MLAI_std = GetParameterFloat("stdlai");

    m_ALA_min = GetParameterFloat("minala");
    m_ALA_max = GetParameterFloat("maxala");
    m_ALA_mod = GetParameterFloat("modala");
    m_ALA_std = GetParameterFloat("stdala");

    try
      {
      m_SampleFile.open(GetParameterString("out").c_str(), std::ofstream::out);
      }
    catch(...)
      {
      itkGenericExceptionMacro(<< "Could not open file " << GetParameterString("out"));
      }

    m_SampleFile << std::setprecision(4);
    m_SampleFile << std::setw(12) << std::left << "MLAI";
    m_SampleFile << std::setw(12) << std::left<< "ALA";
    m_SampleFile << std::setw(12) << std::left    << "CrownCover";
    m_SampleFile << std::setw(12) << std::left    << "HsD";
    m_SampleFile << std::setw(12) << std::left    << "N";
    m_SampleFile << std::setw(12) << std::left    << "Cab";
    m_SampleFile << std::setw(12) << std::left    << "Car";
    m_SampleFile << std::setw(12) << std::left    << "Cdm";
    m_SampleFile << std::setw(12) << std::left    << "CwRel";
    m_SampleFile << std::setw(12) << std::left    << "Cbp";
    m_SampleFile << std::setw(12) << std::left    << "Bs" << std::endl;
    
    auto maxSamples = GetParameterInt("samples");
    auto sampleCount = 0;

    m_RNG = std::mt19937(std::random_device{}());

    otbAppLogINFO("Generating BV samples" << std::endl);
    while(sampleCount < maxSamples)
      {
      this->WriteSample( this->DrawSample() );
      ++sampleCount;
      }
    m_SampleFile.close();

    otbAppLogINFO("" << sampleCount << " samples generated and saved in "
                  << GetParameterString("out") << std::endl);
  }

  double m_LAI_Conv = 10.0;                     
                                                                                            
  double m_MLAI_min = 0.0;                      
  double m_MLAI_max = 5.0;                      
  double m_MLAI_mod = 0.5;                      
  double m_MLAI_std = 1.0;                      
  unsigned short  m_MLAI_nbcl = 6;                       
                                                                                            
  double m_ALA_min = 5.0;                      
  double m_ALA_max = 80.0;                      
  double m_ALA_mod = 40.0;                      
  double m_ALA_std = 20.0;                      
  unsigned short  m_ALA_nbcl = 3.0;                      
                                                                                            
  double m_CrownCover_min = 0.95;                
  double m_CrownCover_max = 1.0;                
  double m_CrownCover_mod = 0.8;                
  double m_CrownCover_std = 0.4;                
  unsigned short  m_CrownCover_nbcl = 1;                 
                                                                                            
  double m_HsD_min = 0.1;                       
  double m_HsD_max = 0.5;                       
  double m_HsD_mod = 0.2;                       
  double m_HsD_std = 0.5;                       
  unsigned short  m_HsD_nbcl = 1;                        
                                                                                            
  double m_N_min = 1.20;                        
  double m_N_max = 2.20;                        
  double m_N_mod = 1.50;                        
  double m_N_std = 0.30;                        
  unsigned short  m_N_nbcl = 3;                          
                                                                                            
  double m_Cab_min = 20.0;                      
  double m_Cab_max = 90.0;                      
  double m_Cab_mod = 45.0;                      
  double m_Cab_std = 30.0;                      
  unsigned short  m_Cab_nbcl = 4;                        
                                                                                            
  double m_Car_min = 0.0;                       
  double m_Car_max = 25.0;                      
  double m_Car_mod = 8.58;                      
  double m_Car_std = 3.95;                      
                                                                                            
  double m_Cdm_min = 0.0030;                    
  double m_Cdm_max = 0.0110;                    
  double m_Cdm_mod = 0.0050;                    
  double m_Cdm_std = 0.0050;                    
  unsigned short  m_Cdm_nbcl = 4;                        
                                                                                            
  double m_CwRel_min = 0.60;                    
  double m_CwRel_max = 0.85;                    
  double m_CwRel_mod = 0.75;                    
  double m_CwRel_std = 0.075;                    
  unsigned short  m_CwRel_nbcl = 4;                      
                                                                                            
  double m_Cbp_min = 0.00;                      
  double m_Cbp_max = 2.00;                      
  double m_Cbp_mod = 0.00;                      
  double m_Cbp_std = 0.30;                      
  unsigned short  m_Cbp_nbcl = 3;                        
                                                                                            
  double m_Bs_min = 0.0;                       
  double m_Bs_max = 1.00;                       
  double m_Bs_mod = 0.5;                       
  double m_Bs_std = 2.00;                       
  unsigned short  m_Bs_nbcl = 4;                         

  // the random number generator
  std::mt19937 m_RNG;

  // the output file
  std::ofstream m_SampleFile;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::BVInputVariableGeneration)
