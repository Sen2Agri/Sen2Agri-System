#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class SpectralFeaturesFunctor
{
public:
  SpectralFeaturesFunctor() : bands(0){}
  SpectralFeaturesFunctor(int bands) : bands(bands){}

  PixelType operator()(PixelType ts, PixelType tf) const
  {
    // get the size of the temporal features raster
    int tfSize = tf.Size();

    // compute the size of the output raster
    int outSize = tfSize * bands;

    // Create the output pixel
    PixelType result(outSize);

    // Loop trough the bands series
    for (int i = 0; i < bands; i++) {
        for (int j = 0; j < tfSize; j++) {
            int imgIndex = static_cast<int>(tf[j]);
            // if the corresponding pixel is nodata replace it with 0.
            result[j * bands + i] = ts[imgIndex * bands + i] < 0 ? 0 : ts[imgIndex * bands + i];
        }
    }

    return result;
  }

  bool operator!=(const SpectralFeaturesFunctor a) const
  {
    return (this->bands != a.bands);
  }

  bool operator==(const SpectralFeaturesFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of each image from the temporal series
  int bands;
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

