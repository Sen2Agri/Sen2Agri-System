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
 
#ifndef __otbSpotMaskFilter_h
#define __otbSpotMaskFilter_h

#include "itkTernaryFunctorImageFilter.h"
#include "otbVectorImage.h"
#include "otbWrapperTypes.h"

#define NODATA          -10000


namespace otb
{


class SpotMaskFunctor
{
public:
    SpotMaskFunctor() {}

    uint8_t operator()(const uint8_t &maskValidity, const uint8_t &maskSaturation, const uint16_t &maskClouds) const
    {
        return (((maskValidity & 0x01) |
                  maskSaturation |
                  maskClouds) == 0) ? 0 : 1;
    }

    bool operator!=(const SpotMaskFunctor) const
    {
        return false;
    }

    bool operator==(const SpotMaskFunctor) const
    {
        return true;
    }
};


/** \class SpotMaskFilter
 * \brief This filter builds the mask for a spot product.
 *
 * \ingroup OTBImageManipulation
 */
class ITK_EXPORT SpotMaskFilter
  : public itk::TernaryFunctorImageFilter<otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt16ImageType, otb::Wrapper::UInt8ImageType, SpotMaskFunctor>
{
public:
  /** Standard class typedefs. */
  typedef SpotMaskFilter                       Self;
  typedef itk::TernaryFunctorImageFilter<otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt16ImageType, otb::Wrapper::UInt8ImageType, SpotMaskFunctor> Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SpotMaskFilter, TernaryFunctorImageFilter);

  /** Template related typedefs */
  typedef otb::Wrapper::UInt8ImageType ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, otb::Wrapper::UInt8ImageType::ImageDimension);

  /** Set the SPOT validity mask. */
  void SetInputValidityMask(const otb::Wrapper::UInt8ImageType *validityMask);

  /** Set the SPOT saturation mask. */
  void SetInputSaturationMask(const otb::Wrapper::UInt8ImageType *saturationMask);

  /** Set the SPOT Cloud mask. */
  void SetInputCloudsMask(const otb::Wrapper::UInt16ImageType *cloudsMask);


protected:
  /** Constructor. */
  SpotMaskFilter();
  /** Destructor. */
  virtual ~SpotMaskFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  SpotMaskFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};
} // end namespace otb

#endif
