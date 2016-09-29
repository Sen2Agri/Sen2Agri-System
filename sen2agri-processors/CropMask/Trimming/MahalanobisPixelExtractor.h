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


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.

  Some parts of this code are derived from ITK. See ITKCopyright.txt
  for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANT2ABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbStreamingStatisticsVectorImageFilter_h
#define __otbStreamingStatisticsVectorImageFilter_h

#include "otbPersistentImageFilter.h"
#include "otbPersistentFilterStreamingDecorator.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkImageRegionSplitter.h"
#include "itkVariableSizeMatrix.h"
#include "itkVariableLengthVector.h"

#include <unordered_map>

template<class TInputImage>
class ITK_EXPORT PersistentMahalanobisPixelExtractorFilter :
  public otb::PersistentImageFilter<TInputImage, TInputImage>
{
public:
  /** Standard Self typedef */
  typedef PersistentMahalanobisPixelExtractorFilter           Self;
  typedef otb::PersistentImageFilter<TInputImage, TInputImage> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(PersistentMahalanobisPixelExtractorFilter, otb::PersistentImageFilter);

  /** Image related typedefs. */
  typedef TInputImage                           ImageType;
  typedef typename ImageType::Pointer           InputImagePointer;
  typedef typename ImageType::RegionType        RegionType;
  typedef typename ImageType::SizeType          SizeType;
  typedef typename ImageType::IndexType         IndexType;
  typedef typename ImageType::PixelType         PixelType;
  typedef typename ImageType::InternalPixelType InternalPixelType;


  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Smart Pointer type to a DataObject. */
  typedef typename itk::DataObject::Pointer DataObjectPointer;
  typedef itk::ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;

  /** Type to use for computations. */
  typedef std::vector<IndexType>                                  IndexVectorType;
  typedef std::unordered_map<short, IndexVectorType>              IndexMapType;

  typedef std::vector<PixelType>                                  PixelVectorType;
  typedef std::unordered_map<short, PixelVectorType>              PixelMapType;



  /** Type of DataObjects used for outputs */
  typedef itk::SimpleDataObjectDecorator<IndexMapType>      IndexMapObjectType;

  /** Return the map of indeces that have passed the mahalanobis filtering*/
  IndexMapType GetIndeces() const
  {
    return this->GetIndOutput()->Get();
  }
  IndexMapObjectType* GetIndecesOutput();
  const IndexMapObjectType* GetIndecesOutput() const;


  /** Add evaluated point. */
  void AddPoint(const IndexType & point, const short cls);

  /** Remove all points */
  void ClearPoints();

  /** Make a DataObject of the correct type to be used as the specified
   * output.
   */
  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);
  using Superclass::MakeOutput;

  virtual void Reset(void);

  virtual void Synthetize(void);

  itkSetMacro(Alpha, double);
  itkGetMacro(Alpha, double);

  itkSetMacro(NbSamples, int);
  itkGetMacro(NbSamples, int);

  itkSetMacro(Seed, int);
  itkGetMacro(Seed, int);

protected:
  PersistentMahalanobisPixelExtractorFilter();

  virtual ~PersistentMahalanobisPixelExtractorFilter() {}

  /** Pass the input through unmodified. Do this by Grafting in the
   *  AllocateOutputs method.
   */
  virtual void AllocateOutputs();

  virtual void GenerateOutputInformation();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Multi-thread version GenerateData. */
  void  ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId);

private:
  PersistentMahalanobisPixelExtractorFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  double              m_Alpha;
  int                 m_NbSamples;
  int                 m_Seed;

  IndexMapType        m_Points;
  PixelMapType        m_Pixels;


}; // end of class PersistentStreamingStatisticsVectorImageFilter

/**===========================================================================*/


template<class TInputImage>
class ITK_EXPORT MahalanobisPixelExtractorFilter :
  public otb::PersistentFilterStreamingDecorator<PersistentMahalanobisPixelExtractorFilter<TInputImage> >
{
public:
  /** Standard Self typedef */
  typedef MahalanobisPixelExtractorFilter Self;
  typedef otb::PersistentFilterStreamingDecorator
  <PersistentMahalanobisPixelExtractorFilter<TInputImage> > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(MahalanobisPixelExtractorFilter, otb::PersistentFilterStreamingDecorator);

  typedef TInputImage                                 InputImageType;
  typedef typename Superclass::FilterType             MahalanobisFilterType;

  /** Type of DataObjects used for outputs */
  typedef typename MahalanobisFilterType::IndexType          IndexType;

  typedef typename MahalanobisFilterType::IndexMapType        IndexMapType;
  typedef typename MahalanobisFilterType::IndexMapObjectType  IndexMapObjectType;

  using Superclass::SetInput;
  void SetInput(InputImageType * input)
  {
    this->GetFilter()->SetInput(input);
  }
  const InputImageType * GetInput()
  {
    return this->GetFilter()->GetInput();
  }

  /** Return the list of indeces that have passes the mahalanobis filtering*/
  IndexMapType GetIndeces() const
  {
    return this->GetFilter()->GetIndecesOutput()->Get();
  }
  IndexMapObjectType* GetIndecesOutput()
  {
    return this->GetFilter()->GetIndecesOutput();
  }
  const IndexMapObjectType* GetIndecesOutput() const
  {
    return this->GetFilter()->GetIndecesOutput();
  }

  /** Add evaluated point. */
  void AddPoint(const IndexType & point, const short cls)
  {
      this->GetFilter()->AddPoint(point, cls);
  }

  /** Remove all points */
  void ClearPoints()
  {
      this->GetFilter()->ClearPoints();
  }


  otbSetObjectMemberMacro(Filter, Alpha, double);
  otbGetObjectMemberMacro(Filter, Alpha, double);

  otbSetObjectMemberMacro(Filter, NbSamples, int);
  otbGetObjectMemberMacro(Filter, NbSamples, int);

  otbSetObjectMemberMacro(Filter, Seed, int);
  otbGetObjectMemberMacro(Filter, Seed, int);


protected:
  /** Constructor */
  MahalanobisPixelExtractorFilter() {}

  /** Destructor */
  virtual ~MahalanobisPixelExtractorFilter() {}

private:
  MahalanobisPixelExtractorFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};

#ifndef OTB_MANUAL_INSTANTIATION
#include "MahalanobisPixelExtractor.hxx"
#endif

#endif
