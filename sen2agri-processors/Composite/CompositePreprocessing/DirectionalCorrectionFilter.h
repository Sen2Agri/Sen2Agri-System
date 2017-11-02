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
 
#ifndef DirectionalCorrectionFilter_H
#define DirectionalCorrectionFilter_H

#include "itkBinaryFunctorImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbWrapperTypes.h"
#include "DirectionalCorrectionFunctor.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "itkNumericTraits.h"
#include "otbImage.h"

//  Now we can declare the filter itself.  It is within the OTB namespace,
//  and we decide to make it use the same image type for both input and
//  output, thus the template declaration needs only one parameter.
//  Deriving from \code{ImageToImageFilter} provides default behavior for
//  several important aspects, notably allocating the output image (and
//  making it the same dimensions as the input).

template <class TImageType, class TOutputImageType>
class ITK_EXPORT DirectionalCorrectionFilter :
  public itk::ImageToImageFilter<TImageType, TOutputImageType>
{
public:
//  Next we have the standard declarations, used for object creation with
//  the object factory:
  typedef DirectionalCorrectionFilter                     Self;
  typedef itk::ImageToImageFilter<TImageType, TOutputImageType> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** Run-time type information */
  itkTypeMacro(DirectionalCorrectionFilter, itk::ImageToImageFilter);

  /* set input xml to read the cloud and water/snow masks */
  void SetInputImage(TImageType inputImg)
  {
    this->SetInput(0, inputImg);
  }

  void SetCloudImage(TImageType inputImg)
  {
    this->SetInput(1, inputImg);
  }

  void SetWaterImage(TImageType inputImg)
  {
    this->SetInput(2, inputImg);
  }

  void SetSnowImage(TImageType inputImg)
  {
    this->SetInput(3, inputImg);
  }

  void SetNdviImage(TImageType inputImg)
  {
    this->SetInput(4, inputImg);
  }

  void SetAnglesImage(TImageType inputImg)
  {
    this->SetInput(5, inputImg);
  }

  void SetScatteringCoefficients(std::vector<ScaterringFunctionCoefficients> &scatCoeffs) {
      m_scatteringCoeffs = scatCoeffs;
  }

  void SetReflQuantifValue(float fQuantifVal) {
      m_fQuantifVal = fQuantifVal;
  }

  virtual void UpdateOutputInformation() ITK_OVERRIDE
  {
      Superclass::UpdateOutputInformation();

      //DirectionalCorrectionFunctorType f;
      this->GetOutput()->SetNumberOfComponentsPerPixel(m_scatteringCoeffs.size());
      //this->GetOutput()->SetNumberOfComponentsPerPixel(f(typename TImageType::PixelType(), typename TImageType::PixelType()).GetSize());
  }

//  Here we declare an alias (to save typing) for the image's pixel type,
//  which determines the type of the threshold value.  We then use the
//  convenience macros to define the Get and Set methods for this parameter.

  typedef typename TImageType::PixelType PixelType;
  typedef typename TOutputImageType::PixelType OutputPixelType;

protected:

  DirectionalCorrectionFilter()
  {
      m_ImageList = ImageListType::New();
      m_Concat = ListConcatenerFilterType::New();
      m_ExtractorList = ExtractROIFilterListType::New();

      this->SetNumberOfRequiredInputs(6);
      m_directionalCorrectionFunctor = FunctorFilterType::New();
  }

//  Now we can declare the component filter types, templated over the
//  enclosing image type:
protected:
  typedef otb::Wrapper::FloatImageType                                         InternalBandImageType;
  typedef otb::ImageList<InternalBandImageType>                                ImageListType;
  typedef otb::ImageListToVectorImageFilter<ImageListType, TImageType >    ListConcatenerFilterType;

  typedef otb::MultiToMonoChannelExtractROI<typename TImageType::InternalPixelType,
                                       InternalBandImageType::PixelType>         ExtractROIFilterType;
  typedef otb::ObjectList<ExtractROIFilterType>                                ExtractROIFilterListType;


  typedef DirectionalCorrectionFunctor <PixelType, OutputPixelType>  DirectionalCorrectionFunctorType;
  typedef itk::UnaryFunctorImageFilter< TImageType, TOutputImageType,
                            DirectionalCorrectionFunctorType > FunctorFilterType;

  void GenerateData() ITK_OVERRIDE
  {
      extractBandsFromImage(this->GetInput(0));
      extractBandsFromImage(this->GetInput(1));
      extractBandsFromImage(this->GetInput(2));
      extractBandsFromImage(this->GetInput(3));
      extractBandsFromImage(this->GetInput(4));
      extractBandsFromImage(this->GetInput(5));

      m_Concat->SetInput(m_ImageList);


      DirectionalCorrectionFunctorType functor;
      functor.Initialize(m_scatteringCoeffs, m_fQuantifVal);
      m_directionalCorrectionFunctor->SetFunctor(functor);

      m_directionalCorrectionFunctor->SetInput(m_Concat->GetOutput());
      m_directionalCorrectionFunctor->UpdateOutputInformation();
      //m_directionalCorrectionFunctor->GetOutput()->SetNumberOfComponentsPerPixel(functor(typename TImageType::PixelType(), typename TImageType::PixelType()).GetSize());
      m_directionalCorrectionFunctor->GetOutput()->SetNumberOfComponentsPerPixel(m_scatteringCoeffs.size());
      m_directionalCorrectionFunctor->GraftOutput(this->GetOutput());
      m_directionalCorrectionFunctor->Update();
      this->GraftOutput(m_directionalCorrectionFunctor->GetOutput());
  }

private:

  int extractBandsFromImage(const TImageType *imageType) {
      int nbBands = imageType->GetNumberOfComponentsPerPixel();
      for(int j=0; j < nbBands; j++)
      {
          typename ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
          extractor->SetInput( imageType );
          extractor->SetChannel( j+1 );
          extractor->UpdateOutputInformation();
          m_ExtractorList->PushBack( extractor );
          m_ImageList->PushBack( extractor->GetOutput() );
      }
      return nbBands;
  }


  DirectionalCorrectionFilter(Self &);   // intentionally not implemented
  void operator =(const Self&);          // intentionally not implemented

  typename FunctorFilterType::Pointer m_directionalCorrectionFunctor;
  std::vector<ScaterringFunctionCoefficients> m_scatteringCoeffs;
  float m_fQuantifVal;

  ImageListType::Pointer              m_ImageList;
  typename ListConcatenerFilterType::Pointer   m_Concat;
  typename ExtractROIFilterListType::Pointer   m_ExtractorList;

//  The component filters are declared as data members, all using the smart
//  pointer types.
};


#endif

