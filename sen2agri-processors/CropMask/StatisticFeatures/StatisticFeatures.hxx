#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkUnaryFunctorImageFilter.h"
#include "otbVectorImage.h"

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class StatisticFeaturesFunctor
{
public:
  StatisticFeaturesFunctor() : bands(0) {}
  StatisticFeaturesFunctor(int bands) : bands(bands){}

  PixelType operator()(PixelType pix) const
  {
    // compute the size of the input pixel
    int pixSize = pix.Size();

    // Create the output pixel
    PixelType result(bands);
    if (pixSize == 0) {
        result.Fill(static_cast<PixelValueType>(0));
        return result;
    }

    // If the input pixel is nodata return nodata
    PixelType nodata(pixSize);
    nodata.Fill(static_cast<PixelValueType>(-10000));

    if (pix == nodata) {
        result.Fill(static_cast<PixelValueType>(-10000));
        return result;
    }

    // If the input pixel is zero return zero
    PixelType zero(pixSize);
    nodata.Fill(static_cast<PixelValueType>(0));

    if (pix == zero) {
        result.Fill(static_cast<PixelValueType>(0));
        return result;
    }

    // Compute the maximum, minimum and mean values
    result[0] = pix[0];
    result[1] = pix[0];
    double avg = 0.0;
    result[2] = static_cast<PixelValueType>(0);
    std::vector<PixelValueType> values(pixSize);

    for (int i = 0; i < pixSize; i++) {
        result[0]  = (result[0] < pix[i] ? pix[i] : result[0] );
        result[1]  = (result[1] > pix[i] ? pix[i] : result[1] );
        avg += pix[i];
        values[i] = pix[i];
    }
    result[2] = static_cast<PixelValueType>(avg / pixSize);

    // compute the median
    std::sort(values.begin(), values.end());
    if (pixSize % 2 == 1) {
        result[3] = values[pixSize / 2];
    } else {
        result[3] = (values[pixSize / 2 - 1] + values[pixSize / 2]) / 2;
    }

    // Compute the standard deviation
    result[4] = static_cast<PixelValueType>(0);
    double stdDev = 0.0;
    for (int i = 0; i < pixSize; i++) {
        stdDev += ((double)pix[i]*pix[i]) - ((double)result[2]*result[2]);
    }
    result[4] = static_cast<PixelValueType>(std::sqrt(stdDev / pixSize));

    return result;
  }

  bool operator!=(const StatisticFeaturesFunctor a) const
  {
    return (this->bands != a.bands);
  }

  bool operator==(const StatisticFeaturesFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of the output raster
  int bands;

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



#endif // TEMPORALRESAMPLING_HXX

