#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "itkUnaryFunctorImageFilter.h"
#include "otbVectorImage.h"

typedef short                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;

#define GETNDVI(b) (static_cast<double>(pix[b]) / 10000.0)

template <typename PixelType>
class DataSmoothingFunctor
{
public:
  DataSmoothingFunctor() : bands(1), lambda(1), weights(0) {}
  DataSmoothingFunctor(int bands, double lambda, std::vector<double> weights) : bands(bands), lambda(lambda), weights(weights){}

  PixelType operator()(PixelType pix) const
  {
    // compute the size of the input pixel
    int pixSize = pix.Size();

    // Create the output pixel
    PixelType result(pix.Size());

    // If the input pixel is nodata return nodata
    PixelType nodata(pixSize);
    nodata.Fill(static_cast<PixelValueType>(-10000));

    if (pix == nodata) {
        result.Fill(static_cast<PixelValueType>(-10000));
        return result;
    }

    // compute the number of images
    int numImages = pixSize / bands;

    // process each band series independently
    for (int band = 0; band < bands; band++) {
        // Create two temporary vectors c and d and the temporary result z.
        std::vector<double> c(numImages);
        std::vector<double> d(numImages);
        std::vector<double> z(numImages);

        // Perform the Whitaker smoothing.
        // The code is inspired from the R language package "ptw"
        int m = numImages-1;
        d[0] = weights[0] + lambda;
        c[0] = -lambda / d[0];
        z[0] = weights[0] * static_cast<double>(pix[band]);

        for (int i = 1; i < m; i++) {
            d[i]= weights[i] + 2 * lambda - c[i-1] * c[i-1] * d[i-1];
            c[i] = -lambda / d[i];
            z[i] = weights[i] * static_cast<double>(pix[i * bands + band]) - c[i-1] * z[i-1];

        }
        d[m] = weights[m] + lambda - c[m-1] * c[m-1] * d[m-1];
        z[m] = (weights[m] * static_cast<double>(pix[m * bands + band]) - c[m-1] * z[m-1]) / d[m];

        result[m * bands + band] = static_cast<PixelValueType>(z[m]);
        for (int i = m-1 ; 0 <= i; i--) {
            z[i] = z[i] / d[i] - c[i] * z[i + 1];
            result[i * bands + band] = static_cast<PixelValueType>(z[i]);
        }
    }

    return result;
  }

  bool operator!=(const DataSmoothingFunctor a) const
  {
    return (this->bands != a.bands || this->lambda != a.lambda || this->weights != a.weights);
  }

  bool operator==(const DataSmoothingFunctor a) const
  {
    return !(*this != a);
  }

protected:
  // the number of bands per image in the temporal series
  int bands;

  // The smoothing parameter of the Whitaker function
  double lambda;

  // The weights associated to the input series
  std::vector<double> weights;
};


#endif // TEMPORALRESAMPLING_HXX

