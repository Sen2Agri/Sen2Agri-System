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
 
#ifndef __otbLandsatMaskFilter_h
#define __otbLandsatMaskFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000


namespace otb
{


template <typename PixelType>
class LandsatMaskFunctor
{
public:
    LandsatMaskFunctor() {}

    PixelType operator()(const PixelType &maskQuality, const PixelType &maskClouds) const
    {
        PixelType result(1);

        // Band 3 from the Quality mask contains the pixel validity
        // Band 1 from the Quality mask contains the saturation
        result[0] = (((static_cast<unsigned short>(maskQuality[2]) & 0x01) |
                       static_cast<unsigned short>(maskQuality[0]) |
                       static_cast<unsigned short>(maskClouds[0])) == 0) ? 0 : 1;

        return result;
    }

    bool operator!=(const LandsatMaskFunctor) const
    {
        return false ;
    }

    bool operator==(const LandsatMaskFunctor) const
    {
        return true;
    }
};


/** \class LandsatMaskFilter
 * \brief This filter builds the mask for a Landsat product.
 *
 * \ingroup OTBImageManipulation
 */
template<class TImage>
class ITK_EXPORT LandsatMaskFilter
  : public itk::BinaryFunctorImageFilter<TImage, TImage, TImage, LandsatMaskFunctor<typename TImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef LandsatMaskFilter                       Self;
  typedef itk::BinaryFunctorImageFilter<TImage, TImage, TImage, LandsatMaskFunctor<typename TImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VectorImageToImagePixelAccessor, BinaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

  /** Set the Landsat quality mask (it contains a band for validity and a band for saturation). */
  void SetInputQualityMask(const TImage *qualityMask);

  /** Set the Landsat Cloud mask. */
  void SetInputCloudsMask(const TImage *cloudsMask);


protected:
  /** Constructor. */
  LandsatMaskFilter();
  /** Destructor. */
  virtual ~LandsatMaskFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  LandsatMaskFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbLandsatMaskFilter.txx"
#endif
#endif
