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

#include "itkUnaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define MAXNDVISLOPE 0
#define MINNDVISLOPE 1
#define MAXNDVI 2
#define MINNDVI 3
#define MAXRED 4
#define INDEXSIZE 5

typedef float PixelValueType;
typedef otb::VectorImage<PixelValueType, 2> ImageType;

template <typename PixelType>
class FeaturesNoInsituFunctor
{
public:
    FeaturesNoInsituFunctor() : m_outImages(5), m_bpi(4), m_id(0)
    {
    }
    FeaturesNoInsituFunctor(int outImages, int bpi, std::vector<int> id)
        : m_outImages(outImages), m_bpi(bpi), m_id(id)
    {
    }

    PixelType operator()(const PixelType &ts) const
    {
        // compute the number of input image
        int numImages = m_id.size();

        PixelType ndvi(numImages);

        auto ok = false;
        for (int imgIndex = 0; imgIndex < numImages; imgIndex++) {
            int b1 = ts[4 * imgIndex];
            int b2 = ts[4 * imgIndex + 1];
            int b3 = ts[4 * imgIndex + 2];
            int b4 = ts[4 * imgIndex + 3];

            if (b1 != -10000 && b2 != -10000 && b3 != -10000 && b4 != -10000) {
                ok = true;

                ndvi[imgIndex] = (std::abs(b3+b2)<0.000001) ? 0 : static_cast<PixelValueType>(b3-b2)/(b3+b2);
            } else {
                ndvi[imgIndex] = -10000;
            }
        }

        // Create the output pixel
        PixelType result(m_outImages * m_bpi);

        if (!ok) {
            result.Fill(static_cast<PixelValueType>(-10000));
            return result;
        }

        // Compute the maximum and minimum NDVI slopes
        // Compute the maximum and minimum NDVI values
        // Compute the maximum Red value
        int index[INDEXSIZE];
        index[MAXNDVISLOPE] = -1;
        index[MINNDVISLOPE] = -1;
        index[MAXNDVI] = 0;
        index[MINNDVI] = 0;
        index[MAXRED] = 0;
        double slope = 0.0;
        double minSlope = 0.0;
        double maxSlope = 0.0;
        double minNDVI = ndvi[0];
        double maxNDVI = ndvi[0];
        double maxRed = ts[1];

        // Get the indices for the required values
        for (int i = 0; i < numImages; i++) {
            if (i > 0 && i < numImages - 1) {
                // compute the slope
                int im1 = i - 1;
                int ip1 = i + 1;
                slope = static_cast<double>(ndvi[ip1] - ndvi[im1]) / (m_id[ip1] - m_id[im1]);
                if (i == 1 || maxSlope < slope) {
                    maxSlope = slope;
                    index[MAXNDVISLOPE] = i;
                }
                if (i == 1 || minSlope > slope) {
                    minSlope = slope;
                    index[MINNDVISLOPE] = i;
                }
            }
            if (maxNDVI < ndvi[i]) {
                maxNDVI = ndvi[i];
                index[MAXNDVI] = i;
            }
            if (minNDVI > ndvi[i]) {
                minNDVI = ndvi[i];
                index[MINNDVI] = i;
            }
            if (maxRed < ts[i * m_bpi + 1]) {
                maxRed = ts[i * m_bpi + 1];
                index[MAXRED] = i;
            }
        }

        for (int i = 0; i < m_outImages; i++) {
            for (int j = 0; j < m_bpi; j++) {
                result[i * m_bpi + j] = (index[i] == -1 ? -10000 : ts[index[i] * m_bpi + j]);
            }
        }

        return result;
    }
    bool operator!=(const FeaturesNoInsituFunctor a) const
    {
        return (this->m_outImages != a.m_outImages) || (this->m_bpi != a.m_bpi) ||
               (this->m_id != a.m_id);
    }

    bool operator==(const FeaturesNoInsituFunctor a) const
    {
        return !(*this != a);
    }

protected:
    // the number of images in the output raster
    int m_outImages;
    // the number of bands per image in the input and output raster
    int m_bpi;
    // the days from epoch corresponding to the input series raster
    std::vector<int> m_id;
};

/** Unary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <typename TFunctor>
class ITK_EXPORT UnaryFunctorImageFilterWithNBands
    : public itk::UnaryFunctorImageFilter<ImageType, ImageType, TFunctor>
{
public:
    typedef UnaryFunctorImageFilterWithNBands Self;
    typedef itk::UnaryFunctorImageFilter<ImageType, ImageType, TFunctor> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Macro defining the type*/
    itkTypeMacro(UnaryFunctorImageFilterWithNBands, SuperClass)

    /** Accessors for the number of bands*/
    itkSetMacro(NumberOfOutputBands, unsigned int)
    itkGetConstMacro(NumberOfOutputBands, unsigned int)

protected:
    UnaryFunctorImageFilterWithNBands()
    {
    }
    virtual ~UnaryFunctorImageFilterWithNBands()
    {
    }

    void GenerateOutputInformation()
    {
        Superclass::GenerateOutputInformation();
        this->GetOutput()->SetNumberOfComponentsPerPixel(m_NumberOfOutputBands);
    }

private:
    UnaryFunctorImageFilterWithNBands(const Self &); // purposely not implemented
    void operator=(const Self &);                     // purposely not implemented

    unsigned int m_NumberOfOutputBands;
};

#endif // TEMPORALRESAMPLING_HXX
