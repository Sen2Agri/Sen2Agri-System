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
 
#ifndef __otbTemporalMergingFilter_h
#define __otbTemporalMergingFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000


namespace otb
{

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

typedef std::vector<ImageInfo> ImageInfoCollection;

template <typename PixelType>
class TemporalMergingFunctor
{
public:
    TemporalMergingFunctor() :  numOutputImages(0), bands(0) {}
    TemporalMergingFunctor(ImageInfoCollection& iInfos, int n) : imgInfos(iInfos), numOutputImages(n), bands(4) {}

    PixelType operator()(const PixelType &pix, const PixelType &mask) const
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
    ImageInfoCollection imgInfos;
    int numOutputImages;
    int bands;
};


/** \class TemporalMergingFilter
 * \brief This filter performs the merge operation on a resampled time series.
 *
 * \ingroup OTBImageManipulation
 */
template<class TImage>
class ITK_EXPORT TemporalMergingFilter
  : public itk::BinaryFunctorImageFilter<TImage,TImage, TImage, TemporalMergingFunctor<typename TImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef TemporalMergingFilter                       Self;
  typedef itk::BinaryFunctorImageFilter<TImage,TImage, TImage, TemporalMergingFunctor<typename TImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(TemporalMergingFilter, BinaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

  /** Accessors for the sensors data */
  itkSetMacro(InputData, ImageInfoCollection);
  itkGetConstMacro(InputData, ImageInfoCollection);

  itkGetConstMacro(OutDays, std::vector<int>);

protected:
  /** Constructor. */
  TemporalMergingFilter();
  /** Destructor. */
  virtual ~TemporalMergingFilter();
  virtual void BeforeThreadedGenerateData();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  TemporalMergingFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  ImageInfoCollection  m_InputData;
  std::vector<int>     m_OutDays;

  // Sort the descriptors based on the aquisition date
  static bool SortImages(const ImageInfo& o1, const ImageInfo& o2) {
      return (o1.day < o2.day) || ((o1.day == o2.day) && (o1.priority > o2.priority));
  }

};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbTemporalMergingFilter.txx"
#endif
#endif
