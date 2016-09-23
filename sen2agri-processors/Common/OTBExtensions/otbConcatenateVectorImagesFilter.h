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
 
#ifndef __otbConcatenateVectorImagesFilter_h
#define __otbConcatenateVectorImagesFilter_h

#include "itkImageToImageFilter.h"
#include "otbVectorImage.h"

namespace otb
{
/** \class ConcatenateVectorsImageFilter
 * \brief This filter concatenates the vector pixels of the input
 * images.
 *
 * \ingroup OTBImageManipulation
 */
template<class TInputImage, class TOutputImage = TInputImage>
class ITK_EXPORT ConcatenateVectorImagesFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef ConcatenateVectorImagesFilter                       Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ConcatenateVectorImagesFilter, ImageToImageFilter);

  /** Template related typedefs */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  typedef typename InputImageType::Pointer  InputImagePointerType;
  typedef typename OutputImageType::Pointer OutputImagePointerType;

  typedef typename InputImageType::PixelType         InputPixelType;

  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename OutputImageType::InternalPixelType OutputInternalPixelType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

protected:
  /** Constructor. */
  ConcatenateVectorImagesFilter();
  /** Destructor. */
  virtual ~ConcatenateVectorImagesFilter();
  virtual void GenerateOutputInformation();
  virtual void BeforeThreadedGenerateData();
  /** Main computation method. */
  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId);
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  ConcatenateVectorImagesFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented
};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbConcatenateVectorImagesFilter.txx"
#endif
#endif
