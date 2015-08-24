#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NOVALUEPIXEL    0.0

typedef otb::VectorImage<float, 2> ImageType;

struct Indeces {
    int minLo;
    int maxLo;
    int minHi;
    int maxHi;
};


template <typename PixelType>
class GapFillingFunctor
{
public:
  GapFillingFunctor() : id(0), od(0), radius(0), bands(0) {}
  GapFillingFunctor(std::vector<int> idDates, std::vector<int> outDates, int r, int b) : id(idDates), od(outDates), radius(r), bands(b) {}

  PixelType operator()(PixelType pix, PixelType mask) const
  {

    // compute the number of images in the output file
    int outImages = od.size();

    // Create the output pixel
    PixelType result(outImages * bands);

    Indeces ind = {0, 0, 0, 0};

    // loop through the output dates.
    for (int outDateIndex = 0; outDateIndex < outImages; outDateIndex++) {
        // get the value of the corresponding output date
        int outDate = od.at(outDateIndex);

        // get the indeces intervals
        ind = getDateIndeces(ind.minLo, outDate);

        // get the id of the first valid date before or equal to the output date
        int beforeId = getPixelDateIndex(ind.maxLo, ind.minLo, mask);

        // get the id of the first valid date after or equal to the output date
        int afterId = getPixelDateIndex(ind.minHi, ind.maxHi, mask);

        // for each band
        for (int band = 0; band < bands; band++) {
            // compute the id of the output pixel
            int outPixelId = outDateIndex * bands + band;

            if (beforeId == -1 && afterId == -1) {
                // if no valid pixel then set a default value (0.0)
                result[outPixelId] = NOVALUEPIXEL;
            } else if (beforeId == -1) {
                // use only the after value
                result[outPixelId] = pix[afterId * bands + band];
            } else if (afterId == -1) {
                // use only the before value
                result[outPixelId] = pix[beforeId * bands + band];
            } else if (beforeId == afterId) {
                // use only the before value which is equal to the after value
                result[outPixelId] = pix[beforeId * bands + band];
            } else {
                // use liniar interpolation to compute the pixel value
                float x1 = id.at(beforeId);
                float y1 = pix[beforeId * bands + band];
                float x2 = id.at(afterId);
                float y2 = pix[afterId * bands + band];

                float a = (y1 - y2) / (x1 - x2);
                float b = y1 - a * x1;

                result[outPixelId] = (a * outDate + b);
            }

        }
    }

    return result;
  }

  bool operator!=(const GapFillingFunctor a) const
  {
    return (this->id != a.id) || (this->od != a.od) || (this->radius != a.radius) || (this->bands != a.bands) ;
  }

  bool operator==(const GapFillingFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // input dates vector
  std::vector<int> id;
  // output dates vector
  std::vector<int> od;
  // the radius for date search
  int radius;
  // the number of bands per image
  int bands;

private:
  // get the indeces of the input dates which are before and after the centerDate and within the radius.
  Indeces getDateIndeces(int startIndex, int centerDate) const {
      Indeces indeces = {-1, -1, -1, -1};

      for (int i=startIndex; i < id.size(); i++) {
          // get input date
          int inputDate = id.at(i);

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
      indeces.minLo = 0;
      indeces.maxHi = id.size() - 1;

      return indeces;
  }

  // Get the date index of the first valid pixel or -1 if none found
  int getPixelDateIndex(int startIndex, int endIndex, const PixelType& mask) const {

      if (startIndex == -1 || endIndex == -1) {
          return -1;
      }

      int index = startIndex;
      bool done = false;

      while (!done) {
          // if the pixel is masked then continue
          if (mask[index] != 0) {
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

