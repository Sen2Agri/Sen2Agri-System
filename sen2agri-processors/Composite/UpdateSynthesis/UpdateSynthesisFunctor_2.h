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
 
#ifndef UPDATESYNTHESISFUNCTOR_H
#define UPDATESYNTHESISFUNCTOR_H

#include <vector>
#include "GlobalDefs.h"

#define WEIGHT_QUANTIF_VALUE    1000

#define DEFAULT_COMPOSITION_QUANTIF_VALUE   10000

#define DATE_NO_DATA            NO_DATA_VALUE
#define WEIGHT_NO_DATA          (NO_DATA_VALUE/WEIGHT_QUANTIF_VALUE)      //  NO_DATA / WEIGHT_QUANTIF_VALUE

class OutFunctorInfos
{
public:
    OutFunctorInfos(int nSize) {
        m_CurrentWeightedReflectances.resize(nSize);
        m_CurrentPixelWeights.resize(nSize);
        m_nCurrentPixelFlag.resize(nSize);
        m_nCurrentPixelWeightedDate.resize(nSize);
    }

    std::vector<float> m_CurrentWeightedReflectances;
    std::vector<float> m_CurrentPixelWeights;
    std::vector<short> m_nCurrentPixelFlag;
    std::vector<short> m_nCurrentPixelWeightedDate;
    //short m_nCurrentPixelFlag;
    //float m_fCurrentPixelWeightedDate;
} ;

template< class TInput, class TOutput>
class UpdateSynthesisFunctor
{
public:
    UpdateSynthesisFunctor();
    UpdateSynthesisFunctor& operator =(const UpdateSynthesisFunctor& copy);
    bool operator!=( const UpdateSynthesisFunctor & other) const;
    bool operator==( const UpdateSynthesisFunctor & other ) const;
    TOutput operator()( const TInput & A );
    void Initialize(const std::vector<int> presenceVect, int nExtractedL2ABandsNo, int nBlueBandIdx,
                    bool bHasAppendedPrevL2ABlueBand, bool bPrevL3ABandsAvailable,
                    int nDate, float fReflQuantifVal);
    int GetNbOfL3AReflectanceBands() { return m_nNbOfL3AReflectanceBands; }
    //int GetNbOfOutputComponents() { return 2*m_nNbOfL3AReflectanceBands+2;}
    int GetNbOfOutputComponents() { return 4*m_nNbOfL3AReflectanceBands;}

    const char * GetNameOfClass() { return "UpdateSynthesisFunctor"; }

private:
    void ResetCurrentPixelValues(const TInput & A, OutFunctorInfos& outInfos);
    int GetAbsoluteL2ABandIndex(int index);
    float GetL2AReflectanceForPixelVal(float fPixelVal);
    void HandleLandPixel(const TInput & A, OutFunctorInfos& outInfos);
    void HandleSnowOrWaterPixel(const TInput & A, OutFunctorInfos& outInfos);
    void HandleCloudOrShadowPixel(const TInput & A, OutFunctorInfos& outInfos);
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsLandPixel(const TInput & A);
    float GetCurrentL2AWeightValue(const TInput & A);
    float GetPrevL3AWeightValue(const TInput & A, int offset);
    short GetPrevL3AWeightedAvDateValue(const TInput & A, int offset);
    float GetPrevL3AReflectanceValue(const TInput & A, int offset);
    short GetPrevL3APixelFlagValue(const TInput & A, int offset);
    int GetBlueBandIndex();
    bool IsNoDataValue(float fValue, float fNoDataValue);

private:
    float m_fReflQuantifValue;
    int m_nCurrentDate;
    int m_nNbL2ABands;

    bool m_bPrevL3ABandsAvailable;
    int m_nNbOfL3AReflectanceBands;


    int m_nL2ABandStartIndex;
    int m_nCloudMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nCurrentL2AWeightBandIndex;
    int m_nPrevL3AWeightBandStartIndex;
    int m_nPrevL3AWeightedAvDateBandIndex;
    int m_nPrevL3AReflectanceBandStartIndex;
    int m_nPrevL3APixelFlagBandIndex;
    int m_nL2ABlueBandIndex;
    int m_nL3ABlueBandIndex;

    bool m_bHasAppendedPrevL2ABlueBand;

    // precomputed value for the reflectance no data (in order to avoid its calculation each pixel)
    //float m_fReflNoDataValue;

    std::vector<int> m_arrL2ABandPresence;

};

#include "UpdateSynthesisFunctor_2.txx"

#endif // UPDATESYNTHESISFUNCTOR_H
