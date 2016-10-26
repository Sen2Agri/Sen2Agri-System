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
 
#ifndef TRIMMING_HXX
#define TRIMMING_HXX

#include "itkImageToImageFilter.h"

#include <unordered_map>

template< typename TInputImage, typename TOutputImage >
class MahalanobisTrimmingFilter:
  public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MahalanobisTrimmingFilter            Self;
  typedef itk::ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef itk::SmartPointer< Self >                            Pointer;
  typedef itk::SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(MahalanobisTrimmingFilter,
               ImageToImageFilter);

  typedef TInputImage                         InputImageType;
  typedef typename InputImageType::Pointer    InputImagePointer;
  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename InputImageType::PixelType  InputImagePixelType;
  typedef typename InputImageType::IndexType  IndexType;
  typedef typename InputImageType::SizeType   SizeType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputImagePixelType;

  typedef std::vector<IndexType>                                  IndexVectorType;
  typedef std::unordered_map<short, IndexVectorType>              IndexMapType;

  void PrintSelf(std::ostream & os, itk::Indent indent) const ITK_OVERRIDE;

  /** Set the points which must be generated in the output raster*/
  void SetPoints(const IndexMapType & points);

protected:
  MahalanobisTrimmingFilter();

//  // Override since the filter needs all the data for the algorithm
//  void GenerateInputRequestedRegion() ITK_OVERRIDE;

//  // Override since the filter produces the entire dataset
//  void EnlargeOutputRequestedRegion(itk::DataObject *output) ITK_OVERRIDE;

  void GenerateData() ITK_OVERRIDE;

private:
  MahalanobisTrimmingFilter(const Self &); //purposely not
                                                      // implemented
  void operator=(const Self &);                       //purposely not

  // implemented

  IndexMapType         m_Points;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "MahalanobisTrimmingFilter.hxx"
#endif


#endif // TRIMMING_HXX

