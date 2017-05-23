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
#define EPS    0.000001

template <typename PixelType>
class CropMaskSupervisedRedEdgeFeaturesFunctor
{
public:

  PixelType operator()(const PixelType &pixel) const
  {
      // in: B2, B4, B5, B6, B7, B8
      // out: RE-NDVI, CHL-RE, PSRI, S2 RE-Position
      int size = pixel.Size();
      int numImages = size / 6;
      PixelType result(numImages * 4);

      int p = 0;
      for (int i = 0; i < size; i += 6, p += 4) {
          int b2 = pixel[i];
          int b4 = pixel[i + 1];
          int b5 = pixel[i + 2];
          int b6 = pixel[i + 3];
          int b7 = pixel[i + 4];
          int b8 = pixel[i + 5];


          if (b2 != NODATA && b4 != NODATA && b5 != NODATA && b6 != NODATA && b7 != NODATA && b8 != NODATA) {
              PixelValueType re_ndvi = std::abs(b8 + b6) < EPS ? NODATA : (b8 - b6) / (b8 + b6);
              PixelValueType chl_re = std::abs(b8) < EPS ? NODATA : b5 / b8;
              PixelValueType psri = std::abs(b5) < EPS ? NODATA : (b4 - b2) / b5;
              PixelValueType s2_re_pos = std::abs(b6 - b5) < EPS ? NODATA : 705 + 35 * (0.5 * (b7 + b4) - b5) / (b6 - b5);

              result[p] = re_ndvi;
              result[p + 1] = chl_re;
              result[p + 2] = psri;
              result[p + 3] = s2_re_pos;
          } else {
              result[p] = NODATA;
              result[p + 1] = NODATA;
              result[p + 2] = NODATA;
              result[p + 3] = NODATA;
        }
      }

      return result;
    }

  bool operator!=(const CropMaskSupervisedRedEdgeFeaturesFunctor &) const
  {
      return false;
  }

  bool operator==(const CropMaskSupervisedRedEdgeFeaturesFunctor &) const
  {
      return true;
  }
};

template<class TImage>
class ITK_EXPORT CropMaskSupervisedRedEdgeFeaturesFilter
  : public itk::UnaryFunctorImageFilter<TImage, TImage, CropMaskSupervisedRedEdgeFeaturesFunctor<typename TImage::PixelType>>
{
public:
  /** Standard class typedefs. */
  typedef CropMaskSupervisedRedEdgeFeaturesFilter                       Self;
  typedef itk::UnaryFunctorImageFilter<TImage, TImage, CropMaskSupervisedRedEdgeFeaturesFunctor<typename TImage::PixelType>> Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CropMaskSupervisedRedEdgeFeaturesFilter, UnaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer           ImagePointerType;

  typedef typename ImageType::PixelType         ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  CropMaskSupervisedRedEdgeFeaturesFilter() { }
  virtual ~CropMaskSupervisedRedEdgeFeaturesFilter() { }

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

protected:
  virtual void GenerateOutputInformation()
  {
      // Call to the superclass implementation
      Superclass::GenerateOutputInformation();

      // initialize the number of channels of the output image
      this->GetOutput()->SetNumberOfComponentsPerPixel(this->GetInput()->GetNumberOfComponentsPerPixel() / 6 * 4);
  }


private:
  CropMaskSupervisedRedEdgeFeaturesFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented
};
