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
 
#ifndef TOTALWEIGHTCOMPUTATION_H
#define TOTALWEIGHTCOMPUTATION_H

#include "otbWrapperTypes.h"
#include "itkBinaryFunctorImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "GlobalDefs.h"
#include "ImageResampler.h"

namespace Functor
{
template< class TPixel>
class TotalWeightCalculationFunctor
{
public:
  TotalWeightCalculationFunctor() {}
  ~TotalWeightCalculationFunctor() {}
  bool operator!=(const TotalWeightCalculationFunctor &) const
  {
    return false;
  }

  bool operator==(const TotalWeightCalculationFunctor & other) const
  {
    return !( *this != other );
  }

  void SetFixedWeight(float weightSensor, float weightDate)
  {
    m_fixedWeight = weightSensor * weightDate;
  }

  inline TPixel operator()(const TPixel & A,
                            const TPixel & B) const
  {
    const float dA = static_cast< float >( A );
    const float dB = static_cast< float >( B );
    if(dA < 0 || dB < 0) {
        return WEIGHT_NO_DATA_VALUE;
    }
    const float totalWeight = fabs(dA * dB * m_fixedWeight);

    return static_cast< TPixel >( totalWeight );
  }
private:
  float m_fixedWeight;
};
}

class TotalWeightComputation
{
public:
    typedef otb::Wrapper::FloatImageType ImageType;
    typedef enum {S2, L8, UNKNOWN} SensorType;
    typedef itk::BinaryFunctorImageFilter< ImageType, ImageType, ImageType,
                              Functor::TotalWeightCalculationFunctor<ImageType::PixelType> > FilterType;
    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<ImageType> WriterType;

    typedef itk::ImageSource<ImageType> ImageSource;
    typedef FilterType::Superclass::Superclass OutImageSource;

public:
    TotalWeightComputation();

    void SetMissionName(std::string &missionName);
    void SetDates(std::string &L2ADate, std::string &L3ADate);
    void SetHalfSynthesisPeriodAsDays(int deltaMax);
    void SetWeightOnDateMin(float fMinWeight);
    void SetAotWeightFile(std::string &aotWeightFileName);
    void SetCloudsWeightFile(std::string &cloudsWeightFileName);
    void SetTotalWeightOutputFileName(std::string &outFileName);

    const char *GetNameOfClass() { return "TotalWeightComputation";}
    OutImageSource::Pointer GetOutputImageSource();

    void WriteToOutputFile();

protected:
    void ComputeTotalWeight();
    void ComputeWeightOnSensor();
    void ComputeWeightOnDate();

protected:
    float m_fWeightOnSensor;
    float m_fWeightOnDate;
    int m_res;

private:
    void BuildOutputImageSource();

    SensorType m_sensorType;
    int m_nDaysTimeInterval;
    int m_nDeltaMax;
    float m_fWeightOnDateMin;
    std::string m_strOutFileName;

    ImageSource::Pointer m_inputReaderAot;
    ImageSource::Pointer m_inputReaderCld;

    FilterType::Pointer m_filter;
    ImageResampler<ImageType, ImageType> m_AotResampler;
    void CheckTolerance();
};

#endif // TOTALWEIGHTCOMPUTATION_H
