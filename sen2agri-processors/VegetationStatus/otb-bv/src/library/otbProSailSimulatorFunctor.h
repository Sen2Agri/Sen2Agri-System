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
#ifndef __OTBPROSAILSIMULATORFUNCTOR_H
#define __OTBPROSAILSIMULATORFUNCTOR_H

#include "otbLeafParameters.h"
#include "otbSailModel.h"
#include "otbProspectModel.h"
#include "otbSatelliteRSR.h"
#include "otbReduceSpectralResponse.h"
#include "otbSolarIrradianceFAPAR.h"

#include "otbBVTypes.h"
namespace otb
{
namespace Functor
{

template <class TSatRSR, unsigned int SimNbBands = 2000>
class ProSailSimulator
{
public:
  /** Standard class typedefs */
  typedef TSatRSR SatRSRType;
  typedef typename SatRSRType::Pointer SatRSRPointerType;
  typedef typename otb::ProspectModel ProspectType;
  typedef typename otb::LeafParameters LeafParametersType;
  typedef typename LeafParametersType::Pointer LeafParametersPointerType;
  typedef typename otb::SailModel SailType;

  typedef typename SatRSRType::PrecisionType PrecisionType;
  typedef std::pair<PrecisionType,PrecisionType> PairType;
  typedef typename std::vector<PairType> VectorPairType;
  typedef otb::SpectralResponse< PrecisionType, PrecisionType>  ResponseType;
  typedef otb::ReduceSpectralResponse < ResponseType,SatRSRType>  ReduceResponseType;
  typedef typename std::vector<PrecisionType> OutputType;
  
  /** Constructor */
  ProSailSimulator() {
    m_SatRSR = SatRSRType::New();
  }

  /** Destructor */
  ~ProSailSimulator() {};
  
  /** Implementation of the () operator*/
  inline
  OutputType operator ()()
  {
    OutputType pix;
    auto nbBands = m_SatRSR->GetNbBands();
    for(size_t i=0;i<nbBands;i++)
      pix.push_back(0.0);

    auto leaf_pars = LeafParametersType::New();
    leaf_pars->SetCab(m_BV[IVNames::Cab]);
    leaf_pars->SetCar(m_BV[IVNames::Car]);
    leaf_pars->SetCBrown(m_BV[IVNames::Cbp]);
    double Cw = m_BV[IVNames::Cdm]/(1.-m_BV[IVNames::CwRel]);
    //TODO : this check should not be needed if the simulations were OK
    if(Cw<0) Cw = 0.0;
    leaf_pars->SetCw(Cw);
    leaf_pars->SetCm(m_BV[IVNames::Cdm]);
    leaf_pars->SetN(m_BV[IVNames::N]);
    m_LAI = m_BV[IVNames::MLAI];
    m_Angl = m_BV[IVNames::ALA];
    m_PSoil = m_BV[IVNames::Bs];
    m_Skyl = 0.3;
    m_HSpot = m_BV[IVNames::HsD];

    auto prospect = ProspectType::New();
    prospect->SetInput(leaf_pars);

    prospect->GenerateData();
    auto refl = prospect->GetReflectance()->GetResponse();
    auto trans = prospect->GetTransmittance()->GetResponse();

    auto sail = SailType::New();
    sail->SetLAI(m_LAI);
    sail->SetAngl(m_Angl);
    sail->SetPSoil(m_PSoil);
    sail->SetSkyl(m_Skyl);
    sail->SetHSpot(m_HSpot);
    sail->SetTTS(m_TTS);
    sail->SetTTO(m_TTO);
    sail->SetPSI(m_PSI);
    sail->SetReflectance(prospect->GetReflectance());
    sail->SetTransmittance(prospect->GetTransmittance());
    sail->Update();

    auto sailSim = sail->GetViewingReflectance()->GetResponse();
    // std::cout << "Sail = " << sailSim.size() << " ------------------------------------- " << std::endl;
    // for(auto i=0; i<sailSim.size(); i++)
    //   std::cout << sailSim[i].first << "\t " << refl[i].second << "\t " << trans[i].second << "\t " << sailSim[i].second << std::endl;

    auto fCover = sail->GetFCoverView();
    
    auto sail_fapar = SailType::New();
    sail_fapar->SetLAI(m_LAI);
    sail_fapar->SetAngl(m_Angl);
    sail_fapar->SetPSoil(m_PSoil);
    sail_fapar->SetSkyl(m_Skyl);
    sail_fapar->SetHSpot(m_HSpot);
    sail_fapar->SetTTS(m_TTS_FAPAR);
    sail_fapar->SetTTO(0.0);
    sail_fapar->SetPSI(0.0);
    sail_fapar->SetReflectance(prospect->GetReflectance());
    sail_fapar->SetTransmittance(prospect->GetTransmittance());
    sail_fapar->Update();

    auto fAPAR = this->ComputeFAPAR(sail_fapar->GetViewingAbsorptance());
    
    VectorPairType hxSpectrum;
    for(size_t i=0;i<SimNbBands;i++)
      {
      PairType resp;
      resp.first = static_cast<PrecisionType>((400.0+i)/1000);
      resp.second = sailSim[i].second;
      hxSpectrum.push_back(resp);
      }

    auto aResponse = ResponseType::New();
    aResponse->SetResponse( hxSpectrum );
    auto  reduceResponse = ReduceResponseType::New();
    reduceResponse->SetInputSatRSR(m_SatRSR);
    reduceResponse->SetInputSpectralResponse( aResponse );
    reduceResponse->SetReflectanceMode(true);
    reduceResponse->CalculateResponse();
    for(size_t i=0;i<m_SatRSR->GetNbBands();i++)
      pix[i] = (*reduceResponse)(i);

    pix.push_back(fCover);
    pix.push_back(fAPAR);
    return pix;
  }

  bool operator !=(const ProSailSimulator& other) const
  {
    return true;
  }

  bool operator ==(const ProSailSimulator& other) const
  {
    return false;
  }

  inline
  void SetRSR(const  SatRSRPointerType rsr)
  {
    m_SatRSR = rsr;
  }

  inline
  SatRSRPointerType GetRSR() const
  {
    return m_SatRSR;
  }

  inline
  void SetBVs(BVType bvmap)
  {
    m_BV = bvmap;
  }

  inline
  void SetParameters(AcquisitionParsType apmap)
  {
    m_TTS = apmap[TTS]; //solar zenith angle
    m_TTO = apmap[TTO]; //observer zenith angle
    m_PSI = apmap[PSI]; //azimuth
    m_TTS_FAPAR = apmap[TTS_FAPAR]; //solar zenith angle for fapar computation

  }
  
protected:

  double ComputeFAPAR(SailType::SpectralResponseType* absorptance){
    double fapar{0};
    double solar_irrad{0};
    for(auto& sip : solar_irradiance_fapar)
      {
      auto l = sip.first;
      auto si = sip.second;
      fapar += (*absorptance)(l)*si;
      solar_irrad += si;
      }
    return fapar/solar_irrad;
  }
  /** Satellite Relative spectral response*/
  SatRSRPointerType m_SatRSR;
  double m_LAI; //leaf area index
  double m_Angl; //average leaf angle
  double m_PSoil; //soil coefficient
  double m_Skyl; //diffuse/direct radiation
  double m_HSpot; //hot spot
  double m_TTS; //solar zenith angle
  double m_TTS_FAPAR; //solar zenith angle for fapar computation
  double m_TTO; //observer zenith angle
  double m_PSI; //azimuth
  BVType m_BV;
};


}
}
#endif
