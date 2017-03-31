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


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __otbListSampleGeneratorRaster_h
#define __otbListSampleGeneratorRaster_h

#include "itkProcessObject.h"
#include "itkListSample.h"
#include "itkPreOrderTreeIterator.h"
#include "itkMersenneTwisterRandomVariateGenerator.h"
#include "otbStreamingManager.h"

namespace otb
{
/** \class ListSampleGeneratorRaster
 *  \brief Produces a ListSample from a VectorImage and a Raster
 *
 *  This filter produces two ListSample for learning and validation of
 *  learning algorithms. The repartition between the learning and validation
 *  ListSample can be adjusted using the SetValidationTrainingProportion()
 *  method.
 *
 *  The size of the training and validation ListSample can be limited using the
 *  SetMaxTrainingSize() and SetMaxValidationSize() methods.
 *
 *  The Raster contins only one band and the value of the pixel represents the class.
 *
 *  The input Raster is supposed to be fully contained within the image extent
 *
 *
 * \ingroup OTBStatistics
 */
template <class TImage, class TRaster>
class ITK_EXPORT ListSampleGeneratorRaster :
  public itk::ProcessObject
{
public:
  /** Standard class typedefs */
  typedef ListSampleGeneratorRaster     Self;
  typedef itk::ProcessObject            Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ListSampleGeneratorRaster, itk::ProcessObject);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef TImage                           ImageType;
  typedef typename ImageType::Pointer      ImagePointerType;
  typedef typename ImageType::IndexType    ImageIndexType;
  typedef typename ImageType::RegionType   ImageRegionType;
  typedef TRaster                          RasterType;
  typedef typename RasterType::Pointer     RasterPointerType;
  typedef typename RasterType::SizeType    RasterSizeType;
  typedef typename RasterType::PixelType   RasterPixelType;

  typedef typename RasterType::IndexType   RasterIndexType;
  typedef typename RasterType::RegionType  RasterRegionType;
  typedef itk::ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;

  /** Streaming manager base class pointer */
  typedef StreamingManager<ImageType>            StreamingManagerType;
  typedef typename StreamingManagerType::Pointer StreamingManagerPointerType;


  /** List to store the pixel values */
  typedef typename ImageType::PixelType           SampleType;
  typedef itk::Statistics::ListSample<SampleType> ListSampleType;
  typedef typename ListSampleType::Pointer        ListSamplePointerType;

  /** List to store the corresponding labels */
  typedef int                                    ClassLabelType;
  typedef itk::FixedArray<ClassLabelType, 1>     LabelType;  //note could be templated by an std:::string
  typedef itk::Statistics::ListSample<LabelType> ListLabelType;
  typedef typename ListLabelType::Pointer        ListLabelPointerType;

  /** Connects the input image for which the sample list is going to be extracted */
  using Superclass::SetInput;
  void SetInput(const ImageType *);
  const ImageType * GetInput() const;

  /** Connects the raster for which the sample list is going to be extracted
   */
  void SetInputRaster(const RasterType *);
  const RasterType * GetInputRaster() const;

  // Build the outputs
  typedef itk::DataObject::Pointer DataObjectPointer;
  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);
  using Superclass::MakeOutput;

  //virtual void Update();

  /** Accessors */
  itkGetConstMacro(MaxTrainingSize, long int);
  itkSetMacro(MaxTrainingSize, long int);
  itkGetConstMacro(MaxValidationSize, long int);
  itkSetMacro(MaxValidationSize, long int);
  itkGetConstMacro(ValidationTrainingProportion, double);
  itkSetClampMacro(ValidationTrainingProportion, double, 0.0, 1.0);
  itkGetConstMacro(BoundByMin, bool);
  itkSetMacro(BoundByMin, bool);
  itkGetConstMacro(NoDataLabel, RasterPixelType);
  itkSetMacro(NoDataLabel, RasterPixelType);

  itkGetConstMacro(NumberOfClasses, unsigned short);
  typedef std::map<ClassLabelType, int> SampleNumberType;

  typedef std::map<ClassLabelType, size_t> ClassesSizeType;

  SampleNumberType GetClassesSamplesNumberTraining(void) const
  {
    return m_ClassesSamplesNumberTraining;
  }

  SampleNumberType GetClassesSamplesNumberValidation(void) const
  {
    return m_ClassesSamplesNumberValidation;
  }

  itkGetConstMacro(ClassMinSize, double);

  /** Returns the Training ListSample as a data object */
  ListSampleType * GetTrainingListSample();

  /** Returns the Trainingn label ListSample as a data object */
  ListLabelType * GetTrainingListLabel();

  /** Returns the label sample list as a data object */
  ListSampleType * GetValidationListSample();

  /** Returns the label sample list as a data object */
  ListLabelType * GetValidationListLabel();

  // Get the map size
  const ClassesSizeType & GetClassesSize() const
  {
    return m_ClassesSize;
  }

  void SetClassesSize(ClassesSizeType classesSize)
  {
      m_ClassesSize = classesSize;
  }

  /** Compute the calss statistics*/
  void GenerateClassStatistics();

protected:
  ListSampleGeneratorRaster();
  virtual ~ListSampleGeneratorRaster() {}
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Triggers the Computation of the sample list */
  virtual void GenerateData(void);

  virtual void GenerateInputRequestedRegion(void);

private:
  ListSampleGeneratorRaster(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented


  void ComputeClassSelectionProbability();


  long int m_MaxTrainingSize; // number of training samples (-1 = no limit)
  long int m_MaxValidationSize; // number of validation samples (-1 = no limit)
  double   m_ValidationTrainingProportion; // proportion of training vs validation
                                           // (0.0 = all training, 1.0 = all validation)

  bool m_BoundByMin; //Bound the number of samples by the class having the fewer
  RasterPixelType m_NoDataLabel; // The value of the NoData pixel from the Raster
  unsigned short m_NumberOfClasses;
  double         m_ClassMinSize;

  ClassesSizeType m_ClassesSize;
  std::map<ClassLabelType, double> m_ClassesProbTraining;
  std::map<ClassLabelType, double> m_ClassesProbValidation;

  std::map<ClassLabelType, int> m_ClassesSamplesNumberTraining; //Just a counter
  std::map<ClassLabelType, int> m_ClassesSamplesNumberValidation; //Just a counter

  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator RandomGeneratorType;
  RandomGeneratorType::Pointer m_RandomGenerator;

  StreamingManagerPointerType m_StreamingManager;
};
} // end of namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbListSampleGeneratorRaster.txx"
#endif

#endif
