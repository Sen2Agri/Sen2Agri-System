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
 
/*=========================================================================

 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) CS SI. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#ifndef __otbLabelMapWithMajorityClassLabelFilter_h
#define __otbLabelMapWithMajorityClassLabelFilter_h
 
#include "itkInPlaceLabelMapFilter.h"

#include <map>

namespace otb
{

/*
 * \class myfilter
 * \brief Classification regularization via a priori segmentation  
 *
 *
 */

template< class TInputImage, class TInputImage2 >
class ITK_EXPORT LabelMapWithMajorityClassLabelFilter:
public itk::InPlaceLabelMapFilter< TInputImage >
{
public:

 /** standard class typedefs */
 typedef LabelMapWithMajorityClassLabelFilter Self;
 typedef itk::InPlaceLabelMapFilter< TInputImage > Superclass;
 typedef itk::SmartPointer< Self > Pointer;
 typedef itk::SmartPointer< const Self > ConstPointer;

 typedef TInputImage InputImageType;
 typedef TInputImage InputImageType2;
 //typedef TOutputImage OutputImageType;

 typedef typename InputImageType::Pointer InputImagePointer;
 typedef typename InputImageType::ConstPointer InputImageConstPointer;
 typedef typename InputImageType::RegionType InputImageRegionType;
 typedef typename InputImageType::PixelType InputImagePixelType;
 typedef typename InputImageType::LabelObjectType LabelObjectType;
 typedef typename LabelObjectType::LabelType  LabelType;
 
 typedef typename InputImageType2::Pointer InputImagePointer2;
 typedef typename InputImageType2::ConstPointer InputImageConstPointer2;
 typedef typename InputImageType2::RegionType InputImageRegionType2;
 typedef typename InputImageType2::PixelType InputImagePixelType2;

 /*typedef typename OutputImageType::Pointer OutputImagePointer;
 typedef typename OutputImageType::ConstPointer OutputImageConstPointer;
 typedef typename OutputImageType::RegionType OutputImageRegionType;
 typedef typename OutputImageType::PixelType OutputImagePixelType;
 typedef typename OutputImageType::IndexType IndexType;
 typedef typename OutputImageType::LabelObjectType OutputLabelObjectType;*/
 
 itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
 itkStaticConstMacro(InputImageDimension2, unsigned int, TInputImage2::ImageDimension);

 /** Object factory management */
 itkNewMacro(Self);

 /** Type macro */
 itkTypeMacro(LabelMapWithMajorityClassLabelFilter, InPlaceLabelMapFilter);

  
  /** Set classif image */
  //itkSetMacro(ClassifImage, typename TInputImage2::Pointer);

void SetClassifImage(const TInputImage2 *input)
{
  // Process object is not const-correct so the const casting is required.
  this->SetNthInput( 1, const_cast< TInputImage2 * >( input ) );
  m_ClassifImage=input;
}

  /** Set noDataSegValue */
  itkSetMacro(NoDataSegValue, LabelType);
  /** Get noDataSegValue */
  itkGetConstReferenceMacro(NoDataSegValue, LabelType);

  /** Set noDataClassifValue */
  itkSetMacro(NoDataClassifValue, InputImagePixelType2);
  /** Get noDataClassifValue */
  itkGetConstReferenceMacro(NoDataClassifValue, InputImagePixelType2);

  /** Set MinArea */
  itkSetMacro(MinArea, int);
  /** Get MinArea */
  itkGetConstReferenceMacro(MinArea, int);
 
 protected:
 LabelMapWithMajorityClassLabelFilter();
 ~LabelMapWithMajorityClassLabelFilter() {}
  //void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,itk::ThreadIdType threadId);
 //virtual void GenerateData();
 void ThreadedProcessLabelObject( LabelObjectType * labelObject );

 private:
 LabelMapWithMajorityClassLabelFilter(const Self &); //purposely not implemented
 void operator=(const Self &); //purposely not implemented

 
 typedef std::map<LabelType,unsigned int>	HistoType;
  
 const TInputImage2 * m_ClassifImage;
 LabelType m_NoDataSegValue;
 InputImagePixelType2 m_NoDataClassifValue;  
 int m_MinArea;


}; // end of class
} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbLabelMapWithMajorityClassLabelFilter.txx"
#endif
 
#endif

