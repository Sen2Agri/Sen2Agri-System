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
 
#pragma once

#include "otbVectorImage.h"

typedef float                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;

#define NODATA -10000

template <typename PixelType>
class ComputeNDVIFunctor
{
public:

  PixelType operator()(PixelType pixel) const
  {
      int numImages = pixel.Size() / 4;

      // Create the output pixel
      PixelType result(numImages);

      for (int imgIndex = 0; imgIndex < numImages; imgIndex++) {
          int b2 = pixel[4 * imgIndex + 1];
          int b3 = pixel[4 * imgIndex + 2];

          if (b2 != NODATA && b3 != NODATA) {
              result[imgIndex] = (std::abs(b3+b2)<0.000001) ? 0 : static_cast<PixelValueType>(b3-b2)/(b3+b2);
          } else {
              result[imgIndex] = NODATA;
        }
      }

      return result;
    }

  bool operator!=(const ComputeNDVIFunctor a) const
  {
      return false;
  }

  bool operator==(const ComputeNDVIFunctor a) const
  {
      return true;
  }
};

template<class TImage>
class ITK_EXPORT ComputeNDVIFilter
  : public itk::UnaryFunctorImageFilter<TImage, TImage, ComputeNDVIFunctor<typename TImage::PixelType>>
{
public:
  /** Standard class typedefs. */
  typedef ComputeNDVIFilter                       Self;
  typedef itk::UnaryFunctorImageFilter<TImage, TImage, ComputeNDVIFunctor<typename TImage::PixelType>> Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ComputeNDVIFilter, UnaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer           ImagePointerType;

  typedef typename ImageType::PixelType         ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  ComputeNDVIFilter() { }
  virtual ~ComputeNDVIFilter() { }

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

protected:
  virtual void GenerateOutputInformation()
  {
      // Call to the superclass implementation
      Superclass::GenerateOutputInformation();

      // initialize the number of channels of the output image
      this->GetOutput()->SetNumberOfComponentsPerPixel(this->GetInput()->GetNumberOfComponentsPerPixel() / 4);
  }


private:
  ComputeNDVIFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented
};
