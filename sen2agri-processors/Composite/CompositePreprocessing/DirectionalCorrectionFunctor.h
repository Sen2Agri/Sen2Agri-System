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
 
#ifndef DIRECTIONALCORRECTIONFUNCTOR_H
#define DIRECTIONALCORRECTIONFUNCTOR_H

class ScaterringFunctionCoefficients
{
public:
    float V0;
    float V1;
    float R0;
    float R1;
};

template< class TInput, class TOutput>
class DirectionalCorrectionFunctor
{
public:
    DirectionalCorrectionFunctor();
    DirectionalCorrectionFunctor& operator =(const DirectionalCorrectionFunctor& copy);
    bool operator!=( const DirectionalCorrectionFunctor & other) const;
    bool operator==( const DirectionalCorrectionFunctor & other ) const;
    TOutput operator()( const TInput & A );
    void Initialize(const std::vector<ScaterringFunctionCoefficients> &coeffs);

    const char * GetNameOfClass() { return "DirectionalCorrectionFunctor"; }

private:
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsLandPixel(const TInput & A);
    float GetCurrentL2AWeightValue(const TInput & A);
    bool IsNoDataValue(float fValue, float fNoDataValue);

private:
    std::vector<ScaterringFunctionCoefficients> m_ScatteringCoeffs;

    int m_nReflBandsCount;

    int m_nCloudMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nNdviBandIdx;
    int m_nSunAnglesBandStartIdx;
    int m_nSensoAnglesBandStartIdx;

    float m_fReflNoDataValue;

};

#include "DirectionalCorrectionFunctor.txx"

#endif // DIRECTIONALCORRECTIONFUNCTOR_H
