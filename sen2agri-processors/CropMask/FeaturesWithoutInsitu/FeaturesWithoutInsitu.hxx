#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define MAXNDVISLOPE        0
#define MINNDVISLOPE        1
#define MAXNDVI             2
#define MINNDVI             3
#define MAXRED              4
#define INDEXSIZE           5

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class FeaturesNoInsituFunctor
{
public:
  FeaturesNoInsituFunctor() : m_outImages(5), m_bpi(4), m_id(0) {}
  FeaturesNoInsituFunctor(int outImages, int bpi, std::vector<int> id) : m_outImages(outImages), m_bpi(bpi), m_id(id) {}

  PixelType operator()(PixelType ndvi, PixelType ts) const
  {
    // compute the number of input images
    int numImages = m_id.size();

    // Create the output pixel
    PixelType result(m_outImages * m_bpi);

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
    int index[INDEXSIZE];
    index[MAXNDVISLOPE]     = -1;
    index[MINNDVISLOPE]     = -1;
    index[MAXNDVI]          = 0;
    index[MINNDVI]          = 0;
    index[MAXRED]           = 0;
    double slope = 0.0;
    double minSlope = 0.0;
    double maxSlope = 0.0;
    double minNDVI = ndvi[0];
    double maxNDVI = ndvi[0];
    double maxRed = ts[1];

    // Get the indeces for the required values
    for (int i = 0; i < numImages; i++) {
        if (i > 0 && i < numImages - 1) {
            //compute the slope
            int im1 = i-1;
            int ip1 = i+1;
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

    // build the result
    for (int i = 0; i< m_outImages; i++) {
        for (int j = 0; j < m_bpi; j++) {
            result[i * m_bpi + j] = (index[i] == -1 ? -10000 : ts[index[i] * m_bpi + j]);
        }
    }

    return result;
  }

  bool operator!=(const FeaturesNoInsituFunctor a) const
  {
    return (this->m_outImages != a.m_outImages) || (this->m_bpi != a.m_bpi) || (this->m_id != a.m_id);
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

