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
 
#ifndef __otbCropTypeFeatureExtractionFilter_h
#define __otbCropTypeFeatureExtractionFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "otbVectorImage.h"
#include "otbTemporalResamplingFilter.h"

#define NODATA          -10000


namespace otb
{

/** \class FeatureTimeSeriesFunctor
 * \brief Functor which builds the CropType features.
 *
 * \ingroup OTBImageManipulation
 */
template <typename PixelType>
class FeatureTimeSeriesFunctor
{
public:
  FeatureTimeSeriesFunctor() : m_OutputBands() { }

  PixelType operator()(const PixelType &rtocr) const
  {
    // Create the output pixel
    PixelType result(m_OutputBands);

    int inputPos = 0;
    int outputPos = 0;
    for (const auto &sd : m_SensorData) {
      for (int i = 0; i < sd.outDates.size(); i++) {
          for (int j = 0; j < sd.bandCount; j++) {
              result[outputPos++] = rtocr[inputPos + j];
          }
          int b1 = rtocr[inputPos];
          int b2 = rtocr[inputPos + 1];
          int b3 = rtocr[inputPos + 2];
          int b4 = rtocr[inputPos + 3];
          float ndvi = (b3==NODATA || b2==NODATA) ? NODATA : ((std::abs(b3+b2)<0.000001) ? 0 : static_cast<float>(b3-b2)/(b3+b2));
          float ndwi = (b4==NODATA || b3==NODATA) ? NODATA : ((std::abs(b4+b3)<0.000001) ? 0 : static_cast<float>(b4-b3)/(b4+b3));
          float brightness = b1==NODATA ? NODATA : std::sqrt(b1*b1 + b2*b2 + b3*b3 + b4*b4);

          result[outputPos++] = ndvi;
          result[outputPos++] = ndwi;
          result[outputPos++] = brightness;

          inputPos += sd.bandCount;
      }
    }

    return result;
  }

  bool operator!=(const FeatureTimeSeriesFunctor a) const
  {
    return (this->m_OutputBands != a.m_OutputBands);
  }

  bool operator==(const FeatureTimeSeriesFunctor a) const
  {
    return !(*this != a);
  }

  // the number of bands of each image from the temporal series
  int m_OutputBands;
  otb::SensorDataCollection m_SensorData;
};



/** \class CropTypeFeatureExtractionFilter
 * \brief This filter extract the Crop Type features.
 *
 * \ingroup OTBImageManipulation
 */
template<class TImage>
class ITK_EXPORT CropTypeFeatureExtractionFilter
  : public itk::UnaryFunctorImageFilter<TImage, TImage, FeatureTimeSeriesFunctor<typename TImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef CropTypeFeatureExtractionFilter                       Self;
  typedef itk::UnaryFunctorImageFilter<TImage, TImage, FeatureTimeSeriesFunctor<typename TImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro(CropTypeFeatureExtractionFilter, UnaryFunctorImageFilter)

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

  void SetSensorData(otb::SensorDataCollection sensorData)
  {
      m_SensorData = std::move(sensorData);
  }

  const otb::SensorDataCollection & GetSensorData() const
  {
      return m_SensorData;
  }

protected:
  /** Constructor. */
  CropTypeFeatureExtractionFilter();
  /** Destructor. */
  virtual ~CropTypeFeatureExtractionFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  CropTypeFeatureExtractionFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  otb::SensorDataCollection m_SensorData;
};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbCropTypeFeatureExtractionFilter.txx"
#endif
#endif
