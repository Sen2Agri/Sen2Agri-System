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
 
#ifndef __otbSentinelMaskFilter_h
#define __otbSentinelMaskFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"
#include "otbWrapperTypes.h"
#include "otbImage.h"

#define NODATA          -10000


namespace otb
{


class SentinelMaskFunctor
{
public:
    SentinelMaskFunctor() {}

    uint8_t operator()(const itk::VariableLengthVector<uint8_t> &maskQuality, const uint8_t &maskClouds) const
    {
        // Band 3 from the Quality mask contains the pixel validity
        // Band 1 from the Quality mask contains the saturation
        return (((maskQuality[2] & 0x01) |
                  maskQuality[0] |
                  maskClouds) == 0) ? 0 : 1;
    }

    bool operator!=(const SentinelMaskFunctor) const
    {
        return false;
    }

    bool operator==(const SentinelMaskFunctor) const
    {
        return true;
    }
};


/** \class SentinelMaskFilter
 * \brief This filter builds the mask for a Sentinel product.
 *
 * \ingroup OTBImageManipulation
 */
class ITK_EXPORT SentinelMaskFilter
  : public itk::BinaryFunctorImageFilter<otb::Wrapper::UInt8VectorImageType, otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt8ImageType, SentinelMaskFunctor>
{
public:
  /** Standard class typedefs. */
  typedef SentinelMaskFilter                       Self;
  typedef itk::BinaryFunctorImageFilter<otb::Wrapper::UInt8VectorImageType, otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt8ImageType, SentinelMaskFunctor> Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro(SentinelMaskFilter, BinaryFunctorImageFilter)

  /** Template related typedefs */
  typedef otb::Wrapper::UInt8ImageType ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, ImageType::ImageDimension);

  /** Set the Sentinel quality mask (it contains a band for validity and a band for saturation). */
  void SetInputQualityMask(const otb::Wrapper::UInt8VectorImageType *qualityMask);

  /** Set the Sentinel Cloud mask. */
  void SetInputCloudsMask(const otb::Wrapper::UInt8ImageType *cloudsMask);


protected:
  /** Constructor. */
  SentinelMaskFilter();
  /** Destructor. */
  virtual ~SentinelMaskFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  SentinelMaskFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};
} // end namespace otb
#endif
