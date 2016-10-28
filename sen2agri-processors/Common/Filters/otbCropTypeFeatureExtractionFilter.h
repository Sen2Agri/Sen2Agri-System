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
  FeatureTimeSeriesFunctor() : m_BandsPerInput(4) { }
  FeatureTimeSeriesFunctor(int bands) : m_BandsPerInput(bands) { }

  PixelType operator()(const PixelType &rtocr) const
  {
    // get the size of the rtocr raster
    int rtocrSize = rtocr.Size();

    // compute the number of images in the input file
    int numImages = rtocrSize / m_BandsPerInput;

    // compute the size of the output
    // for each image there are 7 bands - the initial ones + ndvi, ndwi and brightness
    int outSize = numImages * (m_BandsPerInput + 3);

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the images
    int outIndex = 0;
    for (int i = 0; i < numImages; i++) {
        int inIndex = i * m_BandsPerInput;
        int b1 = rtocr[inIndex];
        int b2 = rtocr[inIndex + 1];
        int b3 = rtocr[inIndex + 2];
        int b4 = rtocr[inIndex + 3];
        // copy the bands to the output
        for (int j = 0; j < m_BandsPerInput; j++) {
            result[outIndex++] = rtocr[inIndex + j];
        }
        // compute the ndvi
        result[outIndex++] = (b3==NODATA || b2==NODATA) ? NODATA : ((std::abs(b3+b2)<0.000001) ? 0 : static_cast<float>(b3-b2)/(b3+b2));
        // compute the ndwi
        result[outIndex++] = (b4==NODATA || b3==NODATA) ? NODATA : ((std::abs(b4+b3)<0.000001) ? 0 : static_cast<float>(b4-b3)/(b4+b3));
        // compute the brightness
        result[outIndex++] = b1==NODATA ? NODATA : std::sqrt(b1*b1 + b2*b2 + b3*b3 + b4*b4);
    }

    return result;
  }

  bool operator!=(const FeatureTimeSeriesFunctor a) const
  {
    return (this->m_BandsPerInput != a.m_BandsPerInput);
  }

  bool operator==(const FeatureTimeSeriesFunctor a) const
  {
    return !(*this != a);
  }

  // the number of bands of each image from the temporal series
  int m_BandsPerInput;
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

  itkSetMacro(BandsPerInput, unsigned int)
  itkGetMacro(BandsPerInput, unsigned int)

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

  unsigned int m_BandsPerInput;
};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbCropTypeFeatureExtractionFilter.txx"
#endif
#endif
