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
 
#include "MaskExtractorFilter.h"

//  Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
//  The constructor sets up the pipeline, which involves creating the
//  stages, connecting them together, and setting default parameters.
//
//  Software Guide : EndLatex

namespace otb
{

//  Software Guide : BeginCodeSnippet
template <class TImageType, class TOutputImageType>
MaskExtractorFilter<TImageType, TOutputImageType>
::MaskExtractorFilter()
{

    m_MaskHandlerFunctor = FunctorFilterType::New();
    this->SetNumberOfRequiredInputs(2);
}

//  Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
//  The \code{GenerateData()} is where the composite magic happens.  First,
//  we connect the first component filter to the inputs of the composite
//  filter (the actual input, supplied by the upstream stage).  Then we
//  graft the output of the last stage onto the output of the composite,
//  which ensures the filter regions are updated.  We force the composite
//  pipeline to be processed by calling \code{Update()} on the final stage,
//  then graft the output back onto the output of the enclosing filter, so
//  it has the result available to the downstream filter.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
template <class TImageType, class TOutputImageType>
void
MaskExtractorFilter<TImageType, TOutputImageType>::
GenerateData()
{
  m_MaskHandlerFunctor->SetInput1(this->GetInput(0));
  m_MaskHandlerFunctor->SetInput2(this->GetInput(1));

  m_MaskHandlerFunctor->UpdateOutputInformation();
  m_MaskHandlerFunctor->GetOutput()->SetNumberOfComponentsPerPixel(3);
  m_MaskHandlerFunctor->GraftOutput(this->GetOutput());
  m_MaskHandlerFunctor->Update();

  this->GraftOutput(m_MaskHandlerFunctor->GetOutput());
}

template <class TImageType, class TOutputImageType>
void
MaskExtractorFilter<TImageType, TOutputImageType>::
SetBitsMask(int cloudMask, int waterMask, int snowMask)
{
    m_MaskHandlerFunctor->GetFunctor().SetBitsMask(cloudMask, waterMask, snowMask);
}

//  Software Guide : EndCodeSnippet

template <class TImageType, class TOutputImageType>
void
MaskExtractorFilter<TImageType, TOutputImageType>::
UpdateOutputInformation()
{
    Superclass::UpdateOutputInformation();

    this->GetOutput()->SetNumberOfComponentsPerPixel(3);
}

//  Software Guide : BeginLatex
//
//  Finally we define the \code{PrintSelf} method, which (by convention)
//  prints the filter parameters.  Note how it invokes the superclass to
//  print itself first, and also how the indentation prefixes each line.
//
//  Software Guide : EndLatex
//
//  Software Guide : BeginCodeSnippet
template <class TImageType, class TOutputImageType>
void
MaskExtractorFilter<TImageType, TOutputImageType>::
PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os
  << indent << "Threshold:" << this->m_Threshold
  << std::endl;
}

} /* end namespace otb */
