#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX

#include "otbVectorImage.h"

typedef short PixelValueType;
typedef otb::VectorImage<PixelValueType, 2> ImageType;

#define GETNDVI(b) (static_cast<double>(pix[b]) / 10000.0)

struct ImageInfo
{
    int index;
    int day;
    int priority;

    ImageInfo(int index, int day, int priority) noexcept : index(index),
                                                           day(day),
                                                           priority(priority)
    {
    }
};

void whit1(double lambda,
           itk::VariableLengthVector<PixelValueType> &values,
           itk::VariableLengthVector<double> &weights,
           itk::VariableLengthVector<PixelValueType> &result)
{
    int numImages = values.Size();
    // Create two temporary vectors c and d and the temporary result z.
    std::vector<double> c(numImages);
    std::vector<double> d(numImages);
    std::vector<double> z(numImages);

    // Perform the Whitaker smoothing.
    // The code is inspired from the R language package "ptw"
    int m = numImages - 1;
    d[0] = weights[0] + lambda;
    c[0] = -lambda / d[0];
    z[0] = weights[0] * static_cast<double>(values[0]);

    for (int i = 1; i < m; i++) {
        d[i] = weights[i] + 2 * lambda - c[i - 1] * c[i - 1] * d[i - 1];
        c[i] = -lambda / d[i];
        z[i] = weights[i] * static_cast<double>(values[i]) - c[i - 1] * z[i - 1];
    }
    d[m] = weights[m] + lambda - c[m - 1] * c[m - 1] * d[m - 1];
    z[m] = (weights[m] * static_cast<double>(values[m]) - c[m - 1] * z[m - 1]) / d[m];

    result[m] = static_cast<PixelValueType>(z[m]);
    for (int i = m - 1; 0 <= i; i--) {
        z[i] = z[i] / d[i] - c[i] * z[i + 1];
        result[i] = static_cast<PixelValueType>(z[i]);
    }
}

template <typename PixelType>
class DataSmoothingFunctor
{
public:
    DataSmoothingFunctor() : outputDates(), bands(1), lambda(1)
    {
    }
    DataSmoothingFunctor(int outputDates,
                         int bands,
                         double lambda,
                         std::vector<ImageInfo> inputImages)
        : outputDates(outputDates),
          bands(bands),
          lambda(lambda),
          inputImages(std::move(inputImages))
    {
    }

    PixelType operator()(const PixelType &pix, const PixelType &mask) const
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

#if 1
        int firstDay = inputImages.front().day;
        int tempSize = inputImages.back().day - firstDay + 1;
        itk::VariableLengthVector<int> indices(tempSize);
        itk::VariableLengthVector<double> weights(tempSize);
        itk::VariableLengthVector<PixelValueType> values(tempSize);
        computeInputIndices(mask, indices);
        for (int i = 0; i < tempSize; i++) {
            if (indices[i] != -1) {
                weights[i] = 1.0;
            } else {
                weights[i] = 0.0;
            }
        }
        itk::VariableLengthVector<PixelValueType> res(tempSize);
        for (int band = 0; band < bands; band++) {
            for (int i = 0; i < tempSize; i++) {
                if (indices[i] != -1) {
                    values[i] = pix[indices[i] * bands + band];
                } else {
                    values[i] = 0;
                }
            }
            whit1(lambda, values, weights, res);
            int pos = 0;
            for (const auto &img : inputImages) {
                result[pos++ * bands + band] = res[img.day - firstDay];
            }
        }
#else
        int outputSize = inputImages.size();
        itk::VariableLengthVector<double> weights(outputSize);
        itk::VariableLengthVector<PixelValueType> values(outputSize);
        for (int i = 0; i < outputSize; i++) {
            if (mask[i]) {
                weights[i] = 0.0;
            } else {
                weights[i] = 1.0;
            }
        }
        itk::VariableLengthVector<PixelValueType> res(outputSize);
        for (int band = 0; band < bands; band++) {
            for (int i = 0; i < outputSize; i++) {
                values[i] = pix[i * bands + band];
            }
            whit1(lambda, values, weights, res);
            for (int i = 0; i < outputSize; i++) {
                result[i * bands + band] = res[i];
            }
        }

#endif
        return result;

//        // process each band series independently
//        for (int band = 0; band < bands; band++) {
//            // Create two temporary vectors c and d and the temporary result z.
//            std::vector<double> c(numImages);
//            std::vector<double> d(numImages);
//            std::vector<double> z(numImages);

//            // Perform the Whitaker smoothing.
//            // The code is inspired from the R language package "ptw"
//            int m = numImages - 1;
//            d[0] = weights[0] + lambda;
//            c[0] = -lambda / d[0];
//            z[0] = weights[0] * static_cast<double>(pix[band]);

//            for (int i = 1; i < m; i++) {
//                d[i] = weights[i] + 2 * lambda - c[i - 1] * c[i - 1] * d[i - 1];
//                c[i] = -lambda / d[i];
//                z[i] =
//                    weights[i] * static_cast<double>(pix[i * bands + band]) - c[i - 1] * z[i - 1];
//            }
//            d[m] = weights[m] + lambda - c[m - 1] * c[m - 1] * d[m - 1];
//            z[m] = (weights[m] * static_cast<double>(pix[m * bands + band]) - c[m - 1] * z[m - 1]) /
//                   d[m];

//            result[m * bands + band] = static_cast<PixelValueType>(z[m]);
//            for (int i = m - 1; 0 <= i; i--) {
//                z[i] = z[i] / d[i] - c[i] * z[i + 1];
//                result[i * bands + band] = static_cast<PixelValueType>(z[i]);
//            }
//        }

//        return result;
    }

    bool operator!=(const DataSmoothingFunctor a) const
    {
        return (this->outputDates != a.outputDates || this->bands != a.bands ||
                this->lambda != a.lambda);
    }

    bool operator==(const DataSmoothingFunctor a) const
    {
        return !(*this != a);
    }

protected:
    int outputDates;
    int bands;
    double lambda;
    std::vector<ImageInfo> inputImages;

    void computeInputIndices(const PixelType &mask, itk::VariableLengthVector<int> &indices) const
    {
        int firstDay = inputImages.front().day;

        indices.Fill(-1);
        for (const auto &img : inputImages) {
            int pos = img.day - firstDay;
            if (indices[pos] == -1 && !mask[img.index]) {
                indices[pos] = img.index;
            }
        }
    }
};

#endif // TEMPORALRESAMPLING_HXX
