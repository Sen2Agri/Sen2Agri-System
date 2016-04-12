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
 
#ifndef WEIGHTONAOT_H
#define WEIGHTONAOT_H

#include "otbWrapperTypes.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkImageSource.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbObjectList.h"
#include "itkUnaryFunctorImageFilter.h"


namespace Functor
{
template< class TInput, class TOutput>
class AotWeightCalculationFunctor
{
public:
    AotWeightCalculationFunctor() {}
    ~AotWeightCalculationFunctor() {}
  bool operator!=( const AotWeightCalculationFunctor & ) const
    {
    return false;
    }
  bool operator==( const AotWeightCalculationFunctor & other ) const
    {
    return !(*this != other);
    }
  void Initialize(int nBand, float fQuantif, float fAotMax, float fMinWeight, float fMaxWeight) {
      m_nBand = nBand;
      m_fAotQuantificationVal = fQuantif;
      m_fAotMax = fAotMax;
      m_fMinWeightAot = fMinWeight;
      m_fMaxWeightAot = fMaxWeight;
  }

  inline TOutput operator()( const TInput & A ) const
  {
      float val = static_cast< float >( A[m_nBand] )/m_fAotQuantificationVal;
      if(val < 0) {
          return 0;
      }

      float fRetAOT;
      if(val < m_fAotMax) {
          fRetAOT = m_fMinWeightAot + (m_fMaxWeightAot - m_fMinWeightAot) * (1.0 - val/m_fAotMax);
      } else {
          fRetAOT = m_fMinWeightAot;
      }

      return fRetAOT;
  }

private:
  int m_nBand;
  float m_fAotQuantificationVal;
  float m_fAotMax;
  float m_fMinWeightAot;
  float m_fMaxWeightAot;
};
}

class WeightOnAOT
{
public:
    typedef otb::Wrapper::FloatVectorImageType ImageType;
    typedef otb::Wrapper::FloatImageType OutImageType;

    typedef itk::ImageSource<ImageType> ImageSource;

    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<OutImageType> WriterType;

    typedef itk::UnaryFunctorImageFilter<ImageType,OutImageType,
                    Functor::AotWeightCalculationFunctor<
                        ImageType::PixelType,
                        OutImageType::PixelType> > FilterType;
    typedef FilterType::Superclass::Superclass::Superclass OutImageSource;

public:
    WeightOnAOT();

    void SetInputFileName(std::string &inputImageStr);
    void SetInputImageReader(ImageSource::Pointer inputReader);
    void SetOutputFileName(std::string &outFile);

    void Initialize(int nBand, float fQuantif, float fAotMax, float fMinWeight, float fMaxWeight);

    const char *GetNameOfClass() { return "WeightOnAOT";}
    OutImageSource::Pointer GetOutputImageSource();
    int GetInputImageResolution();

    void WriteToOutputFile();

private:
    void BuildOutputImageSource();
    ImageSource::Pointer m_inputReader;
    std::string m_outputFileName;

    int m_nBand;
    float m_fAotQuantificationVal;
    float m_fAotMax;
    float m_fMinWeightAot;
    float m_fMaxWeightAot;

    FilterType::Pointer m_filter;
};

#endif // WEIGHTONAOT_H
