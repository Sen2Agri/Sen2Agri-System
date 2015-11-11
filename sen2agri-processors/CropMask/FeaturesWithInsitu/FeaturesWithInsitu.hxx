#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkTernaryFunctorImageFilter.h"
#include "otbVectorImage.h"

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;


template <typename PixelType>
class FeaturesWithInsituFunctor
{
public:
  FeaturesWithInsituFunctor() : bands(0), w(2), delta(500), id(0), tsoil(2000) {}
  FeaturesWithInsituFunctor(int bands, int w, PixelValueType delta, std::vector<int> id, PixelValueType tsoil) : bands(bands), w(w), delta(delta), id(id), tsoil(tsoil) {}

  PixelType operator()(PixelType ndvi,PixelType ndwi, PixelType brightness) const
  {
    // compute the size of the input pixel
    int pixSize = ndvi.Size();

    // The output pixel contains 17 bands built from the ndvi series, 5 bands built from ndwi series and 5 bands built from brightness series
    // Create the output pixel
    PixelType result(bands);

    // If the input pixel is nodata return nodata
    PixelType nodata(pixSize);
    nodata.Fill(static_cast<PixelValueType>(-10000));

    if (ndvi == nodata || ndwi == nodata || brightness == nodata) {
        result.Fill(static_cast<PixelValueType>(-10000));
        return result;
    }

    // Compute the maximum and mean values for ndvi and the maximum, minimum and mean for ndwi and brightness
    result[0] = ndvi[0];
    result[1] = static_cast<PixelValueType>(0);
    double avgNDVI = 0.0;
    result[17] = ndwi[0];
    result[18] = ndwi[0];
    result[19] = static_cast<PixelValueType>(0);
    double avgNDWI = 0.0;
    std::vector<PixelValueType> valuesNDWI(pixSize);

    result[22] = brightness[0];
    result[23] = brightness[0];
    result[24] = static_cast<PixelValueType>(0);
    double avgBrightness = 0.0;
    std::vector<PixelValueType> valuesBrightness(pixSize);

    for (int i = 0; i < pixSize; i++) {
        result[0]  = (result[0]  < ndvi[i] ? ndvi[i] : result[0] );
        avgNDVI += ndvi[i];

        result[17]  = (result[17]  < ndwi[i] ? ndwi[i] : result[17] );
        result[18]  = (result[18]  > ndwi[i] ? ndwi[i] : result[18] );
        avgNDWI += ndwi[i];
        valuesNDWI[i] = ndwi[i];

        result[22]  = (result[22]  < brightness[i] ? brightness[i] : result[22] );
        result[23]  = (result[23]  > brightness[i] ? brightness[i] : result[23] );
        avgBrightness += brightness[i];
        valuesBrightness[i] = brightness[i];
    }
    avgNDVI /= pixSize;
    avgNDWI /= pixSize;
    avgBrightness /= pixSize;
    result[1] = static_cast<PixelValueType>(avgNDVI);
    result[19] = static_cast<PixelValueType>(avgNDWI);
    result[24] = static_cast<PixelValueType>(avgBrightness);

    // Compute the median for ndwi and brightness
    std::sort(valuesNDWI.begin(), valuesNDWI.end());
    std::sort(valuesBrightness.begin(), valuesBrightness.end());
    if (pixSize % 2 == 1) {
        result[20] = valuesNDWI[pixSize / 2];
        result[25] = valuesBrightness[pixSize / 2];
    } else {
        result[20] = (valuesNDWI[pixSize / 2 - 1] + valuesNDWI[pixSize / 2]) / 2;
        result[25] = (valuesBrightness[pixSize / 2 - 1] + valuesBrightness[pixSize / 2]) / 2;
    }

    //Comput the square for the avegare values (used for standard deviation)
    double avgNDVI2 = avgNDVI*avgNDVI;
    double avgNDWI2 = avgNDWI*avgNDWI;
    double avgBrightness2 = avgBrightness*avgBrightness;

    // Compute the standard deviation for ndvi, ndwi and brightness
    result[2] = static_cast<PixelValueType>(0);
    double stdDevNDVI = 0.0;
    result[21] = static_cast<PixelValueType>(0);
    double stdDevNDWI = 0.0;
    result[26] = static_cast<PixelValueType>(0);
    double stdDevBrightness = 0.0;
    for (int i = 0; i < pixSize; i++) {
        stdDevNDVI += ((double)ndvi[i]*ndvi[i] - avgNDVI2);
        stdDevNDWI += ((double)ndwi[i]*ndwi[i] - avgNDWI2);
        stdDevBrightness += ((double)brightness[i]*brightness[i] - avgBrightness2);
    }

    result[2] = static_cast<PixelValueType>(std::sqrt(stdDevNDVI / pixSize));
    result[21] = static_cast<PixelValueType>(std::sqrt(stdDevNDWI / pixSize));
    result[26] = static_cast<PixelValueType>(std::sqrt(stdDevBrightness / pixSize));

    // compute the ndvi transitions only if the size of the input pixel is greater or equal to
    // twice the size of the temporal window
    result[3] = static_cast<PixelValueType>(0);
    result[4] = static_cast<PixelValueType>(0);
    result[5] = static_cast<PixelValueType>(0);
    if (pixSize >= 2 * w) {
        for (int i = 0; i <= pixSize - 2 * w; i++ ) {
            // compute the average of the first part
            double first = 0.0;
            for (int j = i; j <= i + w - 1; j++ ) {
                first += ndvi[j];
            }
            first = first / w;
            // compute the average of the second part
            double second = 0.0;
            for (int j = i + w; j <= i + 2 * w - 1; j++ ) {
                second += ndvi[j];
            }
            second = second / w;

            PixelValueType dif = static_cast<PixelValueType>(first - second);
            // update the max and min
            result[3] = (i == 0 || result[3] < dif ? dif : result[3]);
            result[4] = (i == 0 || result[4] > dif ? dif : result[4]);
        }
        // compute the difference
        result[5] = result[3] - result[4];
    }

    // compute the mbiFeatures associated to the maximum NDVI value:
    result[6] = static_cast<PixelValueType>(0);
    result[7] = static_cast<PixelValueType>(0);
    result[8] = static_cast<PixelValueType>(0);
    if (pixSize >= w) {
        int minIndex = 0, maxIndex = 0;
        for (int i = 0; i <= pixSize - w; i++) {
            // Compute the slice average
            double avg = 0.0;
            for (int j = i; j <= i + w - 1; j++ ) {
                avg += ndvi[j];
            }
            avg = avg / w;

            PixelValueType avgPix = static_cast<PixelValueType>(avg);
            // save the maximum average
            if (avgPix > result[6]) {
                result[6] = avgPix;
                minIndex = i;
                maxIndex = i + w - 1;
            }
        }

        // compute the interval
        double maxAvg = static_cast<double>(result[6]);
        while (result[6] > 0.0 && ndvi[minIndex] >= static_cast<PixelValueType>(maxAvg - delta) && ndvi[minIndex] <= static_cast<PixelValueType>(maxAvg + delta)) {
            minIndex--;
            if (minIndex < 0) {
                minIndex = 0;
                break;
            }
        }
        while (result[6] > 0.0 && ndvi[maxIndex] >= static_cast<PixelValueType>(maxAvg - delta) && ndvi[maxIndex] <= static_cast<PixelValueType>(maxAvg + delta)) {
            maxIndex++;
            if (maxIndex >= pixSize) {
                maxIndex = pixSize - 1;
                break;
            }
        }
        // Computer the difference in days and the area
        result[7] = id[maxIndex] - id[minIndex];
        result[8] = result[6] * result[7];
    }

    // compute the largest increasing and the largers decreasing features
    result[9] = static_cast<PixelValueType>(0);
    result[10] = static_cast<PixelValueType>(0);
    result[11] = static_cast<PixelValueType>(0);
    result[12] = static_cast<PixelValueType>(0);
    result[13] = static_cast<PixelValueType>(0);
    result[14] = static_cast<PixelValueType>(0);

    int minIndexAsc = 0, maxIndexAsc = 0, minIndexDesc = 0, maxIndexDesc = 0;
    // the previous variation is 0 - undefined, 1 - ascending, 2 - descending
    int prevVar = 0;

    for (int i = 1; i < pixSize; i++) {
        bool isDesc = ndvi[i-1] > ndvi[i];
        if (prevVar == 0) {
            // this is the first step. Just save the current indeces
            if (isDesc) {
                // we have a descending slope
                minIndexDesc = i-1;
                maxIndexDesc = i;
                prevVar = 2;
            } else {
                // we have aa ascending slope
                minIndexAsc = i-1;
                maxIndexAsc = i;
                prevVar = 1;
            }
        } else if ( prevVar == 1 && !isDesc) {
            // the slope is still pozitive. Update the ascending maxIndex
            maxIndexAsc = i;
        } else if (prevVar == 2 && isDesc) {
            // the slope is still negative. Update the descending maxIndex
            maxIndexDesc = i;
        } else if (prevVar == 1) {
            // we have ended an increasing interval. Compute the area and decide if it should be saved
            PixelValueType difValue = ndvi[maxIndexAsc] - ndvi[minIndexAsc];
            PixelValueType difTime = static_cast<PixelValueType>( id[maxIndexAsc] - id[minIndexAsc] );
            PixelValueType area = static_cast<PixelValueType>( (difValue * difTime) / 2  );
            if (result[9] < area) {
                result[9] = area;
                result[10] = difTime;
                result[11] = difValue / difTime;
            }

            // set the new slope to descending
            minIndexDesc = i-1;
            maxIndexDesc = i;
            prevVar = 2;
        } else {
            // we have ended an decreasing interval. Compute the area and decide if it should be saved
            PixelValueType difValue = ndvi[minIndexDesc] - ndvi[maxIndexDesc];
            PixelValueType difTime = static_cast<PixelValueType>( id[maxIndexDesc] - id[minIndexDesc] );
            PixelValueType area = static_cast<PixelValueType>( (difValue * difTime) / 2  );
            if (result[12] < area) {
                result[12] = area;
                result[13] = difTime;
                result[14] = difValue / difTime;
            }

            // set the new slope to descending
            minIndexAsc = i-1;
            maxIndexAsc = i;
            prevVar = 1;
        }
    }

    // process the last area
    if (prevVar == 1) {
      // we have ended an increasing interval. Compute the area and decide if it should be saved
      PixelValueType difValue = ndvi[maxIndexAsc] - ndvi[minIndexAsc];
      PixelValueType difTime = static_cast<PixelValueType>( id[maxIndexAsc] - id[minIndexAsc] );
      PixelValueType area = static_cast<PixelValueType>( (difValue * difTime) / 2  );
      if (result[9] < area) {
          result[9] = area;
          result[10] = difTime;
          result[11] = difValue / difTime;
      }
    } else if (prevVar == 2){
      // we have ended an decreasing interval. Compute the area and decide if it should be saved
      PixelValueType difValue = ndvi[minIndexDesc] - ndvi[maxIndexDesc];
      PixelValueType difTime = static_cast<PixelValueType>( id[maxIndexDesc] - id[minIndexDesc] );
      PixelValueType area = static_cast<PixelValueType>( (difValue * difTime) / 2  );
      if (result[12] < area) {
          result[12] = area;
          result[13] = difTime;
          result[14] = difValue / difTime;
      }
    }

    //compute bare soil transitions
    result[15] = static_cast<PixelValueType>(0);
    result[16] = static_cast<PixelValueType>(0);

    // look for a transition
    for (int i = 1; i < pixSize; i++) {
        if (ndvi[i-1] <= tsoil && ndvi[i] >= tsoil) {
            result[15] = static_cast<PixelValueType>(1);
        } else if (ndvi[i-1] >= tsoil && ndvi[i] <= tsoil) {
            result[16] = static_cast<PixelValueType>(1);
        }
    }

    return result;
  }

  bool operator!=(const FeaturesWithInsituFunctor a) const
  {
    return (this->bands != a.bands) || (this->w != a.w) || (this->delta != a.delta) || (this->id != a.id) || (this->tsoil != a.tsoil);
  }

  bool operator==(const FeaturesWithInsituFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands of the output raster
  int bands;
  // the size of the temporal neghbourhood
  int w;
  // the diference delta
  PixelValueType delta;
  // the days from epoch corresponding to the input series raster
  std::vector<int> id;
  // the threshhold for the soil
  PixelValueType tsoil;

};


/** Ternary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <typename TFunctor>
class ITK_EXPORT TernaryFunctorImageFilterWithNBands :
    public itk::TernaryFunctorImageFilter< ImageType, ImageType, ImageType, ImageType, TFunctor >
{
public:
  typedef TernaryFunctorImageFilterWithNBands Self;
  typedef itk::TernaryFunctorImageFilter< ImageType, ImageType, ImageType, ImageType, TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /** Macro defining the type*/
  itkTypeMacro(TernaryFunctorImageFilterWithNBands, SuperClass)
  ;

  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int)
  ;
  itkGetConstMacro(NumberOfOutputBands, unsigned int)
  ;

protected:
  TernaryFunctorImageFilterWithNBands() {}
  virtual ~TernaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  TernaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;


};



#endif // TEMPORALRESAMPLING_HXX

