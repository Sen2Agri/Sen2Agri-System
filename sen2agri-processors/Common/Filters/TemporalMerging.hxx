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
 
#pragma once

#include "itkUnaryFunctorImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"

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

template <typename PixelType, typename MaskType>
class TemporalMergingFunctor
{
public:
    TemporalMergingFunctor() :  numOutputImages(0), bands(0) {}
    TemporalMergingFunctor(std::vector<ImageInfo>& iInfos, int n, int b) : imgInfos(iInfos), numOutputImages(n), bands(b) {}

    PixelType operator()(const PixelType &pix, const MaskType &mask) const
    {
        PixelType result(numOutputImages * bands);

        ImageInfo candidateImage(0,0,0);

        int lastDay = -1;
        int counter = 0;

        for (auto& imgInfo : this->imgInfos) {
            if (lastDay == -1) {
                // save the current entry as candidate day for the current day
                candidateImage = imgInfo;
                lastDay = imgInfo.day;
            } else if (imgInfo.day != lastDay) {
                // the image belongs to a new day. Write the previous candidate to the output
                for (int j = 0; j < bands; j++) {
                    result[counter * bands + j] = pix[candidateImage.index * bands + j];
                }
                counter++;

                // save the current entry as candidate day for the current day
                candidateImage = imgInfo;
                lastDay = imgInfo.day;
            } else {
                // This image has the same day as the candidate image
                // Since the vector is sorted this image has a lower priority
                // It can replace the candidate image if and only if the candidate was marked
                // as invalid while this one is marked as valid
                if (mask[candidateImage.index] != 0 && mask[imgInfo.index] == 0) {
                    candidateImage = imgInfo;
                }
            }
        }

        if (lastDay != -1) {
            // Write the last candidate to the output
            for (int j = 0; j < bands; j++) {
                result[counter * bands + j] = pix[candidateImage.index * bands + j];
            }
        }
        return result;
    }

    bool operator!=(const TemporalMergingFunctor a) const
    {
        return (this->numOutputImages != a.numOutputImages) || (this->bands != a.bands) ;
    }

    bool operator==(const TemporalMergingFunctor a) const
    {
        return !(*this != a);
    }



private:
    std::vector<ImageInfo> imgInfos;
    int numOutputImages;
    int bands;
};

//template <typename TImage>
//using TemporalMergingFilter = itk::UnaryFunctorImageFilter < TImage,
//      TImage, TemporalMergingFunctor<typename TImage::PixelType>;


