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
 
#include "otbLeafParameters.h"
#include "otbSailModel.h"
#include "otbProspectModel.h"
#include "otbSatelliteRSR.h"
#include "otbReduceSpectralResponse.h"
#include "otbBVTypes.h"
int main()
{  
  typedef double PrecisionType;
  typedef otb::SatelliteRSR<PrecisionType, PrecisionType>  SatRSRType;
  typedef typename otb::ProspectModel ProspectType;
  typedef typename otb::LeafParameters LeafParametersType;
  typedef typename otb::SailModel SailType;
  typedef typename SatRSRType::PrecisionType PrecisionType;
  typedef std::pair<PrecisionType,PrecisionType> PairType;
  typedef typename std::vector<PairType> VectorPairType;
  typedef otb::SpectralResponse< PrecisionType, PrecisionType>  ResponseType;
  typedef otb::ReduceSpectralResponse < ResponseType,SatRSRType>  ReduceResponseType;
  typedef typename std::vector<PrecisionType> OutputType;
  const unsigned int simNbBands{2000};
  const PrecisionType vLAI = 3.728;
  const PrecisionType vALA = 59.755;
  const PrecisionType vHsD = 0.186;
  const PrecisionType vN = 1.494;
  const PrecisionType vCab = 64.632;
  const PrecisionType vCar = vCab*0.25;
  const PrecisionType vCdm = 0.008;
  const PrecisionType vCwRel = 0.733;
  const PrecisionType vCw = vCdm/(1.-vCwRel);
  const PrecisionType vCbp = 0.075;
  const PrecisionType vBs = 0.729;
  constexpr PrecisionType rad2deg = 180.0/3.1415; 
  const PrecisionType vVZ = 0.305*rad2deg;
  const PrecisionType vVAz = 5.023*rad2deg;
  const PrecisionType vSZ = 0.647*rad2deg;
  const PrecisionType vSAz = 2.428*rad2deg;
  const PrecisionType vSkyl = 0.3;
  const std::string dataFileName{"/tmp/simulations.dat"};
  const std::string rsrFileName{"/home/inglada/Dev/otb-bv/data/formosat2.rsr"};
  const unsigned int satNbBands{5};
  OutputType pix;
  for(size_t i=0;i<satNbBands;i++)
    pix.push_back(0.0);
  auto m_LP = LeafParametersType::New();
  m_LP->SetCab(vCab);
  m_LP->SetCar(vCar);
  m_LP->SetCBrown(vCbp);
  m_LP->SetCw(vCw);
  m_LP->SetCm(vCdm);
  m_LP->SetN(vN);
  auto prospect = ProspectType::New();
  prospect->SetInput(m_LP);
  prospect->GenerateData();
  auto refl = prospect->GetReflectance()->GetResponse();
  auto trans = prospect->GetTransmittance()->GetResponse();
  auto sail = SailType::New();
  sail->SetLAI(vLAI);
  sail->SetAngl(vALA);
  sail->SetPSoil(vBs);
  sail->SetSkyl(vSkyl);
  sail->SetHSpot(vHsD);
  sail->SetTTS(vSZ);
  sail->SetTTO(vVZ);
  sail->SetPSI(vSAz-vVAz);
  sail->SetReflectance(prospect->GetReflectance());
  sail->SetTransmittance(prospect->GetTransmittance());
  sail->Update();
  auto sailSim = sail->GetViewingReflectance()->GetResponse();
  VectorPairType hxSpectrum;
  for(size_t i=0;i<simNbBands;i++)
    {
    PairType resp;
    resp.first = static_cast<PrecisionType>((400.0+i)/1000);
    resp.second = sailSim[i].second;
    hxSpectrum.push_back(resp);
    }
  auto aResponse = ResponseType::New();
  aResponse->SetResponse( hxSpectrum );
  auto myRSR = SatRSRType::New();
  myRSR->SetNbBands(satNbBands);
  myRSR->SetSortBands(false);
  myRSR->Load(rsrFileName);
  auto reduceResponse = ReduceResponseType::New();
  reduceResponse->SetInputSatRSR(myRSR);
  reduceResponse->SetInputSpectralResponse(aResponse);
  reduceResponse->SetReflectanceMode(true);
  reduceResponse->CalculateResponse();
  for(size_t i=0;i<satNbBands;i++)
    std::cout << (*reduceResponse)(i) << std::endl;
  std::ofstream dataFile(dataFileName, std::ios_base::out);
  dataFile.setf( std::ios::fixed, std:: ios::floatfield );
  dataFile.precision(4);
  for(size_t i=0; i<simNbBands; i++)
    {
    dataFile << hxSpectrum[i].first << " ";
    dataFile << refl[i].second << " ";
    dataFile << sailSim[i].second << " ";
    dataFile << (*myRSR)(hxSpectrum[i].first, 0) << " ";    //B
    dataFile << (*myRSR)(hxSpectrum[i].first, 1) << " ";    //G
    dataFile << (*myRSR)(hxSpectrum[i].first, 2) << " ";    //R
    dataFile << (*myRSR)(hxSpectrum[i].first, 3) << " ";    //NIR
    dataFile << (*myRSR)(hxSpectrum[i].first, 4) << " ";    //Pan
    dataFile << std::endl;
    }
  dataFile.close();
  return EXIT_SUCCESS;
}
/* Plot with
plot "/tmp/simulations.dat" using 1:2 with lines t "Prospect reflectance", "/tmp/simulations.dat" using 1:3 with lines t "Sail reflectance", "/tmp/simulations.dat" using 1:4 with lines t "B1", "/tmp/simulations.dat" using 1:5 with lines t "B2", "/tmp/simulations.dat" using 1:6 with lines t "B3", "/tmp/simulations.dat" using 1:7 with lines t "B4"
gnuplot> plot "/tmp/simulations.dat" using 1:2 with lines t "Prospect reflectance", "/tmp/simulations.dat" using 1:3 with lines t "Sail reflectance", "/tmp/simulations.dat" using 1:4 with lines t "B1", "/tmp/simulations.dat" using 1:5 with lines t "B2", "/tmp/simulations.dat" using 1:6 with lines t "B3", "/tmp/simulations.dat" using 1:7 with lines t "B4", "/tmp/simulations.dat" using 1:8 with lines t "Pan"
*/
