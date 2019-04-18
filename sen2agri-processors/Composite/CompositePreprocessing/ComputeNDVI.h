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
 
#ifndef COMPUTE_NDVI_H
#define COMPUTE_NDVI_H

#include "otbImage.h"
#include "otbWrapperTypes.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "GlobalDefs.h"
#include "ImageResampler.h"
#include "MetadataHelper.h"

class ComputeNDVI
{
public:
    typedef float                                                                   PixelType;
    typedef otb::Image<PixelType, 2>                                                OutputImageType;
    typedef otb::Wrapper::Int16VectorImageType                                      ImageType;
    typedef otb::ImageFileReader<ImageType>                                         ImageReaderType;
    typedef itk::UnaryFunctorImageFilter<ImageType,OutputImageType,
                    NdviFunctor<
                        ImageType::PixelType,
                        OutputImageType::PixelType> > FilterType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
    
public:
    ComputeNDVI();
    void DoInit(std::string &xml, int nRes);
    OutputImageType::Pointer DoExecute();
    const char * GetNameOfClass() { return "ComputeNDVI"; }
    void WriteToOutputFile();

private:
    std::string                         m_inXml;
    int m_nResolution;
    FilterType::Pointer m_Functor;
    ImageResampler<OutputImageType, OutputImageType> m_ResampledBandsExtractor;

    std::unique_ptr<MetadataHelper<short>> m_pHelper;

};

#endif // COMPUTE_NDVI
