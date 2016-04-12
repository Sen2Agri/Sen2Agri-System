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
 
#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

typedef float                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class TemporalFeaturesNoInsituFunctor
{
public:
  TemporalFeaturesNoInsituFunctor() : bands(0), id(0) {}
  TemporalFeaturesNoInsituFunctor(int bands, std::vector<int> id) : bands(bands), id(id) {}

  PixelType operator()(PixelType ndvi, PixelType ts) const
  {
    // compute the number of images
    int numImages = id.size();

    // compute the number of bands per image in the time serie
    int bpi = ts.Size() / ndvi.Size();

    // Create the output pixel
    PixelType result(bands);

    // If the input pixel is nodata return nodata
    PixelType nodata(numImages);
    nodata.Fill(static_cast<PixelValueType>(-10000));

    if (ndvi == nodata) {
        result.Fill(static_cast<PixelValueType>(-10000));
        return result;
    }

    // Compute the maximum and minimum NDVI slopes
    // Compute the maximum and minimum NDVI values
    // Compute the maximum Red value
    result[0] = static_cast<PixelValueType>(0);
    result[1] = static_cast<PixelValueType>(0);
    result[2] = static_cast<PixelValueType>(0);
    result[3] = static_cast<PixelValueType>(0);
    result[4] = static_cast<PixelValueType>(0);
    double slope = 0.0;
    double minSlope = 0.0;
    double maxSlope = 0.0;
    double minNDVI = ndvi[0];
    double maxNDVI = ndvi[0];
    double maxRed = ts[1];

    for (int i = 0; i < numImages; i++) {
        if (i > 0 && i < numImages - 1) {
            //compute the slope
            int im1 = i-1;
            int ip1 = i+1;
            slope = (ndvi[ip1] - ndvi[im1]) / (id[ip1] - id[im1]);
            if (i == 1 || maxSlope < slope) {
                maxSlope = slope;
                result[0] = static_cast<PixelValueType>(i);
            }
            if (i == 1 || minSlope > slope) {
                minSlope = slope;
                result[1] = static_cast<PixelValueType>(i);
            }
        }
        if (maxNDVI < ndvi[i]) {
            maxNDVI = ndvi[i];
            result[2] = static_cast<PixelValueType>(i);
        }
        if (minNDVI > ndvi[i]) {
            minNDVI = ndvi[i];
            result[3] = static_cast<PixelValueType>(i);
        }
        if (maxRed < ts[i * bpi + 1]) {
            maxRed = ts[i * bpi + 1];
            result[4] = static_cast<PixelValueType>(i);
        }
    }

    return result;
  }

  bool operator!=(const TemporalFeaturesNoInsituFunctor a) const
  {
    return (this->bands != a.bands) || (this->id != a.id);
  }

  bool operator==(const TemporalFeaturesNoInsituFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of the output raster
  int bands;
  // the days from epoch corresponding to the input series raster
  std::vector<int> id;
};


/** Binary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <typename TFunctor>
class ITK_EXPORT BinaryFunctorImageFilterWithNBands :
    public itk::BinaryFunctorImageFilter< ImageType, ImageType,
                                          ImageType, TFunctor >
{
public:
  typedef BinaryFunctorImageFilterWithNBands Self;
  typedef itk::BinaryFunctorImageFilter< ImageType, ImageType,
                                         ImageType, TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /** Macro defining the type*/
  itkTypeMacro(BinaryFunctorImageFilterWithNBands, SuperClass)
  ;

  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int)
  ;
  itkGetConstMacro(NumberOfOutputBands, unsigned int)
  ;

protected:
  BinaryFunctorImageFilterWithNBands() {}
  virtual ~BinaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  BinaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;
};


#endif // TEMPORALRESAMPLING_HXX

