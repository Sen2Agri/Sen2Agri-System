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
 
#ifndef SPOTMASKEXTRACTORFILTER_H
#define SPOTMASKEXTRACTORFILTER_H

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


//  Software Guide : BeginLatex
//
//  The composite filter we will build combines three filters: a gradient
//  magnitude operator, which will calculate the first-order derivative of
//  the image; a thresholding step to select edges over a given strength;
//  and finally a rescaling filter, to ensure the resulting image data is
//  visible by scaling the intensity to the full spectrum of the output
//  image type.
//
//  Since this filter takes an image and produces another image (of
//  identical type), we will specialize the ImageToImageFilter:
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
//  Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
//  Next we include headers for the component filters:
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include "itkBinaryFunctorImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbWrapperTypes.h"
#include "SpotMaskHandlerFunctor.h"
#include "libgen.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
//  Software Guide : EndCodeSnippet

#include "itkNumericTraits.h"
#include "otbImage.h"

//  Software Guide : BeginLatex
//
//  Now we can declare the filter itself.  It is within the OTB namespace,
//  and we decide to make it use the same image type for both input and
//  output, thus the template declaration needs only one parameter.
//  Deriving from \code{ImageToImageFilter} provides default behavior for
//  several important aspects, notably allocating the output image (and
//  making it the same dimensions as the input).
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
namespace otb
{

template <class TImageType, class TOutputImageType>
class ITK_EXPORT SPOTMaskExtractorFilter :
  public itk::ImageToImageFilter<TImageType, TOutputImageType>
{
public:
//  Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
//  Next we have the standard declarations, used for object creation with
//  the object factory:
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
  typedef SPOTMaskExtractorFilter                     Self;
  typedef itk::ImageToImageFilter<TImageType, TOutputImageType> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;
//  Software Guide : EndCodeSnippet

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** Run-time type information */
  itkTypeMacro(SPOTMaskExtractorFilter, itk::ImageToImageFilter);

  /* set input xml to read the cloud and water/snow masks */
  void SetInputXml(std::string xml);

  /** Display */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;
  //virtual void UpdateOutputInformation() ITK_OVERRIDE;

//  Software Guide : BeginLatex
//
//  Here we declare an alias (to save typing) for the image's pixel type,
//  which determines the type of the threshold value.  We then use the
//  convenience macros to define the Get and Set methods for this parameter.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
  typedef typename TImageType::PixelType PixelType;
  typedef typename TOutputImageType::PixelType OutputPixelType;

  itkGetMacro(Threshold, PixelType);
  itkSetMacro(Threshold, PixelType);

//  Software Guide : EndCodeSnippet

protected:

  SPOTMaskExtractorFilter();

//  Software Guide : BeginLatex
//
//  Now we can declare the component filter types, templated over the
//  enclosing image type:
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
protected:
  typedef SpotMaskHandlerFunctor <PixelType, PixelType, OutputPixelType> SpotMaskHandlerFunctorType;
  typedef itk::BinaryFunctorImageFilter< TImageType, TImageType, TOutputImageType,
                            SpotMaskHandlerFunctorType > FunctorFilterType;

  typedef otb::ImageList<TImageType>  ImageListType;
  typedef ImageListToVectorImageFilter<ImageListType,
                                       PixelType >                   ListConcatenerFilterType;

  typedef itk::ThresholdImageFilter<TImageType> ThresholdType;
  typedef itk::GradientMagnitudeImageFilter<TImageType, TImageType>
  GradientType;
  typedef itk::RescaleIntensityImageFilter<TImageType, TImageType>
  RescalerType;
//  Software Guide : EndCodeSnippet

  virtual void GenerateData() ITK_OVERRIDE;


private:

  SPOTMaskExtractorFilter(Self &);   // intentionally not implemented
  void operator =(const Self&);          // intentionally not implemented

//  Software Guide : BeginLatex
//
//  The component filters are declared as data members, all using the smart
//  pointer types.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
  typename GradientType::Pointer m_GradientFilter;
  typename ThresholdType::Pointer m_ThresholdFilter;
  typename RescalerType::Pointer m_RescaleFilter;

  typename FunctorFilterType::Pointer          m_SpotMaskHandlerFunctor;
  SpotMaskHandlerFunctorType          m_Functor;


  PixelType m_Threshold;
};

} /* namespace otb */

#endif

