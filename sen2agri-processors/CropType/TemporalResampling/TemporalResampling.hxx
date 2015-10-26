#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NOVALUEPIXEL    -10000.0

typedef otb::VectorImage<short, 2> ImageType;

typedef std::vector<int>                RasterDates;

struct Indeces {
    int minLo;
    int maxLo;
    int minHi;
    int maxHi;
};

struct SensorData {
    // the name of the sensor (for information only)
    std::string sensorName;
    // the dates of the input
    RasterDates inDates;
    // the offset for the first band in the input raster
    int         offset;
    // the number of output raster created for this sensor
    int         outNum;
    // the date of the first output raster that should be created for this sensor
    int         outStart;
    // the date of the last output raster that should be created for this sensor
    int         outEnd;
};


template <typename PixelType>
class GapFillingFunctor
{
public:
  GapFillingFunctor() : od(0), radius(0), bands(0) {}
  GapFillingFunctor(std::vector<SensorData>& inData, RasterDates& outDates, int r, int b) : inputData(inData), od(outDates), radius(r), bands(b) {radius = 365000;}

  PixelType operator()(PixelType pix, PixelType mask) const
  {

    // compute the number of image dates.
    int outImages = od.size();

    // Create the output pixel
    int outSize = 0;
    for (const SensorData& sd : inputData) {
        outSize += sd.outNum;
    }
    PixelType result(outSize * bands);
    int outPixelId = 0;


    // loop through the output dates.
    for (int outDateIndex = 0; outDateIndex < outImages; outDateIndex++) {
        // get the value of the corresponding output date
        int outDate = od.at(outDateIndex);

        // Build the output for each sensor, if needed
        for (const SensorData& sd : inputData) {
            if (sd.outStart <= outDate && sd.outEnd >= outDate) {
                Indeces ind = {0, 0, 0, 0};
                // get the indeces intervals
                ind = getDateIndeces(sd, outDate);

                // get the id of the first valid date before or equal to the output date
                int beforeId = getPixelDateIndex(ind.maxLo, ind.minLo, mask, sd);

                // get the id of the first valid date after or equal to the output date
                int afterId = getPixelDateIndex(ind.minHi, ind.maxHi, mask, sd);

                // build the offseted ids
                int beforeRasterId = beforeId + sd.offset;
                int afterRasterId = afterId + sd.offset;

                // for each band
                for (int band = 0; band < bands; band++) {

                    if (beforeId == -1 && afterId == -1) {
                        // if no valid pixel then set a default value (0.0)
                        result[outPixelId] = NOVALUEPIXEL;
                    } else if (beforeId == -1) {
                        // use only the after value
                        result[outPixelId] = pix[afterRasterId * bands + band];
                    } else if (afterId == -1) {
                        // use only the before value
                        result[outPixelId] = pix[beforeRasterId * bands + band];
                    } else if (beforeId == afterId) {
                        // use only the before value which is equal to the after value
                        result[outPixelId] = pix[beforeRasterId * bands + band];
                    } else {
                        // use liniar interpolation to compute the pixel value
                        float x1 = sd.inDates.at(beforeId);
                        float y1 = pix[beforeRasterId * bands + band];
                        float x2 = sd.inDates.at(afterId);
                        float y2 = pix[afterRasterId * bands + band];

                        float a = (y1 - y2) / (x1 - x2);
                        float b = y1 - a * x1;

                        result[outPixelId] = (short)(a * outDate + b);
                    }
                    outPixelId++;

                }
            }
        }

    }

    return result;
  }

  bool operator!=(const GapFillingFunctor a) const
  {
    return (this->od != a.od) || (this->radius != a.radius) || (this->bands != a.bands) ;
  }

  bool operator==(const GapFillingFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // input data
  std::vector<SensorData> inputData;
  // output dates vector
  std::vector<int> od;
  // the radius for date search
  int radius;
  // the number of bands per image
  int bands;

private:
  // get the indeces of the input dates which are before and after the centerDate and within the radius.
  Indeces getDateIndeces(const SensorData& sd, int centerDate) const {
      Indeces indeces = {-1, -1, -1, -1};

      for (int i=0; i < sd.inDates.size(); i++) {
          // get input date
          int inputDate = sd.inDates.at(i);

          if (inputDate > centerDate + radius) {
              // all future dates are after the center date.
              break;
          }

          if (inputDate < centerDate - radius) {
              // the date is before the interval
              continue;
          }

          // The first time this point is reached will offer the index for the minimum value of the minLo value
          if (indeces.minLo == -1) {
              indeces.minLo = i;
          }

          if (inputDate == centerDate) {
              // this date is at least the end of the before interval and the begining of the after interval
              indeces.maxLo = i;
              indeces.minHi = i;
              indeces.maxHi = i;
          } else if (inputDate < centerDate) {
              // this is a candidate maxLo Date
              indeces.maxLo = i;
          } else {
              // this can be the minHi if not already set in the previous step (at date equality)
              if (indeces.minHi == -1) {
                  indeces.minHi = i;
              }

              // this is a candidate maxHi
              indeces.maxHi = i;
          }
      }

      return indeces;
  }

  // Get the date index of the first valid pixel or -1 if none found
  int getPixelDateIndex(int startIndex, int endIndex, const PixelType& mask, const SensorData& sd ) const {

      if (startIndex == -1 || endIndex == -1) {
          return -1;
      }

      int index = startIndex;
      bool done = false;

      while (!done) {
          // if the pixel is masked then continue
          if (mask[index+sd.offset] != 0) {
              // if it was the last index then stop and return -1
              if (index == endIndex) {
                  index = -1;
                  done = true;
              } else if (startIndex < endIndex) {
                  index ++;
              } else {
                  index --;
              }

          } else {
              // the search is done
              done = true;
          }
      }

      return index;
  }

};


/** Binary functor image filter which produces a vector image with a
* number of bands different from the input images */
class ITK_EXPORT BinaryFunctorImageFilterWithNBands :
    public itk::BinaryFunctorImageFilter< ImageType, ImageType,
                                          ImageType, GapFillingFunctor<ImageType::PixelType> >
{
public:
  typedef BinaryFunctorImageFilterWithNBands Self;
  typedef itk::BinaryFunctorImageFilter< ImageType, ImageType,
                                         ImageType, GapFillingFunctor<ImageType::PixelType> > Superclass;
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

