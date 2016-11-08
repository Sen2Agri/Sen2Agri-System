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
 
#ifndef __otbTemporalResamplingFilter_h
#define __otbTemporalResamplingFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000


namespace otb
{

typedef std::vector<int> RasterDates;

struct Indices
{
    int minLo;
    int maxLo;
    int minHi;
    int maxHi;
};

struct SensorData
{
    // the name of the sensor (for information only)
    std::string sensorName;
    // the dates of the input
    RasterDates inDates;
    // the dates for the output
    RasterDates outDates;

    int bandCount;

    bool operator ==(const SensorData a) const {
        return !(*this != a);
    }

    bool operator !=(const SensorData a) const {
        return (this->sensorName != a.sensorName) || (this->inDates != a.inDates) || (this->outDates != a.outDates);
    }
};

typedef std::vector<SensorData> SensorDataCollection;

template <typename PixelType, typename MaskType, typename OutputType>
class GapFillingFunctor
{
public:
    GapFillingFunctor() : radius(0) {}
    GapFillingFunctor(std::vector<SensorData> &inData, int r)
        : inputData(inData), radius(r)
    {
        radius = 365000;

        outputSize = 0;
        for (const SensorData &sd : inputData) {
            outputSize += sd.outDates.size() * sd.bandCount;
        }
    }

    GapFillingFunctor(std::vector<SensorData> &inData)
        : inputData(inData), radius(365000)
    {
        outputSize = 0;
        for (const SensorData &sd : inputData) {
            outputSize += sd.outDates.size() * sd.bandCount;
        }
    }

    OutputType operator()(const PixelType &pix, const MaskType &mask) const
    {
        // Create the output pixel
        OutputType result(outputSize);

        int sensorStart = 0;
        int outPixelId = 0;
        int offset = 0;
        for (const auto &sd : inputData) {
            for (auto outDate : sd.outDates) {
                Indices ind = { 0, 0, 0, 0 };
                // get the indices intervals
                ind = getDateIndices(sd, outDate);

                // get the id of the first valid date before or equal to the output date
                int beforeId = getPixelDateIndex(ind.maxLo, ind.minLo, mask, offset);

                // get the id of the first valid date after or equal to the output date
                int afterId = getPixelDateIndex(ind.minHi, ind.maxHi, mask, offset);

                // for each band
                for (int band = 0; band < sd.bandCount; band++) {

                    if (beforeId == -1 && afterId == -1) {
                        // if no valid pixel then set a default value (0.0)
                        result[outPixelId] = NODATA;
                    } else if (beforeId == -1) {
                        // use only the after value
                        result[outPixelId] = pix[sensorStart + afterId * sd.bandCount + band];
                    } else if (afterId == -1) {
                        // use only the before value
                        result[outPixelId] = pix[sensorStart + beforeId * sd.bandCount + band];
                    } else if (beforeId == afterId) {
                        // use only the before value which is equal to the after value
                        result[outPixelId] = pix[sensorStart + beforeId * sd.bandCount + band];
                    } else {
                        // use linear interpolation to compute the pixel value
                        float x1 = sd.inDates[beforeId];
                        float y1 = pix[sensorStart + beforeId * sd.bandCount + band];
                        float x2 = sd.inDates[afterId];
                        float y2 = pix[sensorStart + afterId * sd.bandCount + band];
                        float a = (y1 - y2) / (x1 - x2);
                        float b = y1 - a * x1;

                        result[outPixelId] = static_cast<typename OutputType::ValueType>(a * outDate + b);
                    }
                    outPixelId++;
                }
            }
            offset += sd.inDates.size();
            sensorStart += sd.inDates.size() * sd.bandCount;
        }

        return result;
    }

    bool operator!=(const GapFillingFunctor a) const
    {
        return (this->radius != a.radius);
    }

    bool operator==(const GapFillingFunctor a) const
    {
        return !(*this != a);
    }

protected:
    // input data
    std::vector<SensorData> inputData;
    // the radius for date search
    int radius;
    // the number of output bands
    int outputSize;

private:
    // get the indices of the input dates which are before and after the centerDate and within the
    // radius.
    Indices
    getDateIndices(const SensorData &sd, int centerDate) const
    {
        Indices indices = { -1, -1, -1, -1 };

        for (int i = 0; i < sd.inDates.size(); i++) {
            // get input date
            int inputDate = sd.inDates[i];

            if (inputDate > centerDate + radius) {
                // all future dates are after the center date.
                break;
            }

            if (inputDate < centerDate - radius) {
                // the date is before the interval
                continue;
            }

            // The first time this point is reached will offer the index for the minimum value of
            // the minLo value
            if (indices.minLo == -1) {
                indices.minLo = i;
            }

            if (inputDate == centerDate) {
                // this date is at least the end of the before interval and the begining of the
                // after interval
                indices.maxLo = i;
                indices.minHi = i;
                indices.maxHi = i;
            } else if (inputDate < centerDate) {
                // this is a candidate maxLo Date
                indices.maxLo = i;
            } else {
                // this can be the minHi if not already set in the previous step (at date equality)
                if (indices.minHi == -1) {
                    indices.minHi = i;
                }

                // this is a candidate maxHi
                indices.maxHi = i;
            }
        }

        return indices;
    }

    // Get the date index of the first valid pixel or -1 if none found
    int getPixelDateIndex(int startIndex, int endIndex, const PixelType &mask, int offset)
        const
    {

        if (startIndex == -1 || endIndex == -1) {
            return -1;
        }

        int index = startIndex;
        bool done = false;

        while (!done) {
            // if the pixel is masked then continue
            if (mask[index + offset] != 0) {
                // if it was the last index then stop and return -1
                if (index == endIndex) {
                    index = -1;
                    done = true;
                } else if (startIndex < endIndex) {
                    index++;
                } else {
                    index--;
                }

            } else {
                // the search is done
                done = true;
            }
        }

        return index;
    }
};



/** \class TemporalResamplingFilter
 * \brief This filter performs gap filling with or without resampling on a image time series.
 *
 * \ingroup OTBImageManipulation
 */
template<class TInputImage, class TMask, class TOutputImage>
class ITK_EXPORT TemporalResamplingFilter
  : public itk::BinaryFunctorImageFilter<TInputImage, TMask, TInputImage, GapFillingFunctor<typename TInputImage::PixelType, typename TMask::PixelType, typename TOutputImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef TemporalResamplingFilter                       Self;
  typedef itk::BinaryFunctorImageFilter<TInputImage, TMask, TInputImage, GapFillingFunctor<typename TInputImage::PixelType, typename TMask::PixelType, typename TOutputImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(TemporalResamplingFilter, BinaryFunctorImageFilter);

  /** Accessors for the sensors data */
  void SetInputData(const SensorDataCollection _arg)
  {
      m_InputData = std::move(_arg);
  }

  const SensorDataCollection & GetInputData() const
  {
      return m_InputData;
  }

  /** Set the input raster. */
  void SetInputRaster(const TInputImage *raster);

  /** Set the input mask. */
  void SetInputMask(const TMask *mask);

protected:
  /** Constructor. */
  TemporalResamplingFilter();
  /** Destructor. */
  virtual ~TemporalResamplingFilter();
  virtual void BeforeThreadedGenerateData();
  virtual void GenerateOutputInformation();

private:
  TemporalResamplingFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  SensorDataCollection  m_InputData;

};

/** Binary functor image filter which produces a vector image with a
* number of bands different from the input images */
template<typename Input1ImageType, typename Input2ImageType, typename OutputImageType, typename Functor>
class ITK_EXPORT BinaryFunctorImageFilterWithNBands
    : public itk::BinaryFunctorImageFilter<Input1ImageType,
                                           Input2ImageType,
                                           OutputImageType,
                                           Functor >
{
public:
    typedef BinaryFunctorImageFilterWithNBands Self;
    typedef itk::BinaryFunctorImageFilter<Input1ImageType,
                                          Input2ImageType,
                                          OutputImageType,
                                          Functor > Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Macro defining the type*/
    itkTypeMacro(BinaryFunctorImageFilterWithNBands, SuperClass);

    /** Accessors for the number of bands*/
    itkSetMacro(NumberOfOutputBands, unsigned int);
    itkGetConstMacro(NumberOfOutputBands, unsigned int);

protected:
    BinaryFunctorImageFilterWithNBands()
    {
    }
    virtual ~BinaryFunctorImageFilterWithNBands()
    {
    }

    void GenerateOutputInformation()
    {
        Superclass::GenerateOutputInformation();
        this->GetOutput()->SetNumberOfComponentsPerPixel(m_NumberOfOutputBands);
    }

private:
    BinaryFunctorImageFilterWithNBands(const Self &); // purposely not implemented
    void operator=(const Self &); // purposely not implemented

    unsigned int m_NumberOfOutputBands;
};

} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbTemporalResamplingFilter.txx"
#endif
#endif
