#ifndef FEATUREEXTRACTION_HXX
#define FEATUREEXTRACTION_HXX

#include "itkUnaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000
#define SCALE           10000

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class FeatureTimeSeriesFunctor
{
public:
  FeatureTimeSeriesFunctor() : m_bands(4){}
  FeatureTimeSeriesFunctor(int bands) : m_bands(bands){}

  PixelType operator()(PixelType rtocr) const
  {
    // get the size of the rtocr raster
    int rtocrSize = rtocr.Size();

    // compute the number of images in the input file
    int numImages = rtocrSize / m_bands;

    // compute the size of the output
    // for each image there are 7 bands - the initial ones + ndvi, ndwi and brightness
    int outSize = numImages * (m_bands + 3);

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the images
    int outIndex = 0;
    for (int i = 0; i < numImages; i++) {
        int inIndex = i * m_bands;
        int b1 = rtocr[inIndex];
        int b2 = rtocr[inIndex + 1];
        int b3 = rtocr[inIndex + 2];
        int b4 = rtocr[inIndex + 3];
        // copy the bands to the output
        for (int j = 0; j < m_bands; j++) {
            result[outIndex++] = rtocr[inIndex + j];
        }
        // compute the ndvi
        result[outIndex++] = (b3==NODATA || b2==NODATA) ? NODATA : ((std::abs(b3+b2)<0.000001) ? 0 : SCALE * (b3-b2)/(b3+b2));
        // compute the ndwi
        result[outIndex++] = (b4==NODATA || b3==NODATA) ? NODATA : ((std::abs(b4+b3)<0.000001) ? 0 : SCALE * (b4-b3)/(b4+b3));
        // compute the brightness
        result[outIndex++] = b1==NODATA ? NODATA : std::sqrt(b1*b1 + b2*b2 + b3*b3 + b4*b4);
    }

    return result;
  }

  bool operator!=(const FeatureTimeSeriesFunctor a) const
  {
    return (this->m_bands != a.m_bands);
  }

  bool operator==(const FeatureTimeSeriesFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of each image from the temporal series
  int m_bands;
};

template <typename PixelType>
class NDVIFunctor
{
public:
  NDVIFunctor() : m_bands(4){}
  NDVIFunctor(int bands) : m_bands(bands){}

  PixelType operator()(PixelType rtocr) const
  {
    // get the size of the rtocr raster
    int rtocrSize = rtocr.Size();

    // compute the number of images in the input file
    int numImages = rtocrSize / m_bands;

    // compute the size of the output
    // one band for each input image
    int outSize = numImages ;

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the images
    int outIndex = 0;
    for (int i = 0; i < numImages; i++) {
        int inIndex = i * m_bands;
        int b2 = rtocr[inIndex + 1];
        int b3 = rtocr[inIndex + 2];
        // compute the ndvi
        result[outIndex++] = (b3==NODATA || b2==NODATA) ? NODATA : ((std::abs(b3+b2)<0.000001) ? 0 : SCALE * (b3-b2)/(b3+b2));
    }

    return result;
  }

  bool operator!=(const NDVIFunctor a) const
  {
    return (this->m_bands != a.m_bands);
  }

  bool operator==(const NDVIFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of each image from the temporal series
  int m_bands;
};

template <typename PixelType>
class NDWIFunctor
{
public:
  NDWIFunctor() : m_bands(4){}
  NDWIFunctor(int bands) : m_bands(bands){}

  PixelType operator()(PixelType rtocr) const
  {
    // get the size of the rtocr raster
    int rtocrSize = rtocr.Size();

    // compute the number of images in the input file
    int numImages = rtocrSize / m_bands;

    // compute the size of the output
    // one band for each input image
    int outSize = numImages ;

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the images
    int outIndex = 0;
    for (int i = 0; i < numImages; i++) {
        int inIndex = i * m_bands;
        int b3 = rtocr[inIndex + 2];
        int b4 = rtocr[inIndex + 3];
        // compute the ndwi
        result[outIndex++] = (b4==NODATA || b3==NODATA) ? NODATA : ((std::abs(b4+b3)<0.000001) ? 0 : SCALE * (b4-b3)/(b4+b3));
    }

    return result;
  }

  bool operator!=(const NDWIFunctor a) const
  {
    return (this->m_bands != a.m_bands);
  }

  bool operator==(const NDWIFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of each image from the temporal series
  int m_bands;
};

template <typename PixelType>
class BrightnessFunctor
{
public:
  BrightnessFunctor() : m_bands(4){}
  BrightnessFunctor(int bands) : m_bands(bands){}

  PixelType operator()(PixelType rtocr) const
  {
    // get the size of the rtocr raster
    int rtocrSize = rtocr.Size();

    // compute the number of images in the input file
    int numImages = rtocrSize / m_bands;

    // compute the size of the output
    // one band for each input image
    int outSize = numImages ;

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the images
    int outIndex = 0;
    for (int i = 0; i < numImages; i++) {
        int inIndex = i * m_bands;
        int b1 = rtocr[inIndex];
        int b2 = rtocr[inIndex + 1];
        int b3 = rtocr[inIndex + 2];
        int b4 = rtocr[inIndex + 3];
        // compute the brightness
        result[outIndex++] = b1==NODATA ? NODATA : std::sqrt(b1*b1 + b2*b2 + b3*b3 + b4*b4);
    }

    return result;
  }

  bool operator!=(const BrightnessFunctor a) const
  {
    return (this->m_bands != a.m_bands);
  }

  bool operator==(const BrightnessFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of each image from the temporal series
  int m_bands;
};

/** Unary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <typename TFunctor>
class ITK_EXPORT UnaryFunctorImageFilterWithNBands :
    public itk::UnaryFunctorImageFilter< ImageType, ImageType, TFunctor >
{
public:
  typedef UnaryFunctorImageFilterWithNBands Self;
  typedef itk::UnaryFunctorImageFilter< ImageType, ImageType, TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /** Macro defining the type*/
  itkTypeMacro(UnaryFunctorImageFilterWithNBands, SuperClass)
  ;

  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int)
  ;
  itkGetConstMacro(NumberOfOutputBands, unsigned int)
  ;

protected:
  UnaryFunctorImageFilterWithNBands() {}
  virtual ~UnaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  UnaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;
};

#endif // FEATUREEXTRACTION_HXX

