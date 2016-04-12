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

#include "otbVectorImage.h"

typedef float PixelValueType;
typedef otb::VectorImage<PixelValueType, 2> ImageType;
typedef otb::VectorImage<short, 2> MaskImageType;

#define GETNDVI(b) (static_cast<double>(pix[b]))

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
           const itk::VariableLengthVector<PixelValueType> &values,
           const itk::VariableLengthVector<double> &weights,
           itk::VariableLengthVector<PixelValueType> &result)
{
    int numImages = values.Size();
    // Create two temporary vectors c and d and the temporary result z.
    std::vector<double> c(numImages);
    std::vector<double> d(numImages);
    std::vector<double> z(numImages);

    const double eps = 0.0001;

    // Perform the Whitaker smoothing.
    // The code is inspired from the R language package "ptw"
    int m = numImages - 1;
    d[0] = weights[0] + lambda;
    c[0] = fabs(d[0]) > eps ? -lambda / d[0] : 0.0;
    z[0] = weights[0] * static_cast<double>(values[0]);

    for (int i = 1; i < m; i++) {
        d[i] = weights[i] + 2 * lambda - c[i - 1] * c[i - 1] * d[i - 1];
        c[i] = fabs(d[i]) > eps ? -lambda / d[i] : 0.0;
        z[i] = weights[i] * static_cast<double>(values[i]) - c[i - 1] * z[i - 1];
    }
    d[m] = weights[m] + lambda - c[m - 1] * c[m - 1] * d[m - 1];
    z[m] = fabs(d[m]) > eps ? (weights[m] * static_cast<double>(values[m]) - c[m - 1] * z[m - 1]) / d[m] : 0.0;

    result[m] = static_cast<PixelValueType>(z[m]);
    for (int i = m - 1; 0 <= i; i--) {
        z[i] = (fabs(d[i]) > eps ? z[i] / d[i] : 0.0) - c[i] * z[i + 1];
        result[i] = static_cast<PixelValueType>(z[i]);
    }
}

template <typename PixelType, typename MaskPixelType>
class DataSmoothingFunctor
{
public:
    DataSmoothingFunctor() : bands(1), lambda(1), outputDates()
    {
    }
    DataSmoothingFunctor(int bands,
                         double lambda,
                         std::vector<int> outputDates,
                         std::vector<ImageInfo> inputImages)
        : bands(bands),
          lambda(lambda),
          outputDates(std::move(outputDates)),
          inputImages(std::move(inputImages))
    {
    }

    PixelType operator()(const PixelType &pix, const MaskPixelType &mask) const
    {
        // compute the size of the input pixel
        int pixSize = pix.Size();

        // Create the output pixel
        PixelType result(outputDates.size() * bands);

        // If the input pixel is nodata return nodata
        PixelType nodata(pixSize);
        nodata.Fill(static_cast<PixelValueType>(-10000));

        if (pix == nodata) {
            result.Fill(static_cast<PixelValueType>(-10000));
            return result;
        }

        int firstDay = inputImages.front().day;
        int tempSize = inputImages.back().day - firstDay + 1;
        itk::VariableLengthVector<int> indices(tempSize);
        itk::VariableLengthVector<double> weights(tempSize);
        itk::VariableLengthVector<PixelValueType> values(tempSize);

        indices.Fill(-1);
        for (const auto &img : inputImages) {
            int pos = img.day - firstDay;
            if (indices[pos] == -1 && !mask[img.index]) {
                indices[pos] = img.index;
            }
        }

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
            auto pos = 0;
            for (auto date : outputDates) {
                result[pos++ * bands + band] = res[date - firstDay];
            }
        }

        return result;
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
    int bands;
    double lambda;
    std::vector<int> outputDates;
    std::vector<ImageInfo> inputImages;
};

#endif // TEMPORALRESAMPLING_HXX
