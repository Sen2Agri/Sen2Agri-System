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
 
#ifndef CUTTINGIMAGEHANDLER_H
#define CUTTINGIMAGEHANDLER_H

#include "otbVectorImage.h"
#include "otbWrapperTypes.h"

//Transform
#include "otbImageFileWriter.h"
#include "otbExtractROI.h"

template <typename TInput1, typename TInput2>
class CuttingImageHandler
{
public:

    typedef otb::ImageFileWriter<TInput2> WriterType;
    typedef itk::ImageSource<TInput1> ImageSource;
    typedef itk::ImageSource<TInput2> ResampledImageSource;
    typedef otb::ExtractROI<typename TInput1::InternalPixelType,
                            typename TInput2::PixelType>     ExtractROIFilterType;

    typedef typename itk::ImageSource<TInput2> OutImageSource;

public:
    CuttingImageHandler() {
        m_outForcedWidth = -1;
        m_outForcedHeight = -1;
    }

    void SetOutputFileName(std::string &outFile) {
        m_outputFileName = outFile;
    }

    void SetInputImageReader(typename ImageSource::Pointer inputImgReader, int width, int height) {
        if (inputImgReader.IsNull())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        m_inputImgReader = inputImgReader;
        m_outForcedWidth = width;
        m_outForcedHeight = height;
    }

    const char *GetNameOfClass() { return "CuttingImageHandler";}

    typename OutImageSource::Pointer GetOutputImageSource() {
        BuildOutputImage();
        if(m_extractor.IsNull()) {
            return m_inputImgReader;
        }

        return (typename OutImageSource::Pointer)m_extractor;
    }

    void WriteToOutputFile() {
        if(!m_outputFileName.empty())
        {
            typename WriterType::Pointer writer;
            writer = WriterType::New();
            writer->SetFileName(m_outputFileName);
            writer->SetInput(GetOutputImageSource()->GetOutput());
            try
            {
                writer->Update();
                typename TInput2::Pointer image = m_inputImgReader->GetOutput();
                typename TInput2::SpacingType spacing = image->GetSpacing();
                typename TInput2::PointType origin = image->GetOrigin();
                std::cout << "==============CUT IMAGE===================" << std::endl;
                std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
                std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
                std::cout << "Size : " << image->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             image->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                typename TInput2::SpacingType outspacing = m_extractor->GetOutput()->GetSpacing();
                typename TInput2::PointType outorigin = m_extractor->GetOutput()->GetOrigin();
                std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
                std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
                std::cout << "Size : " << m_extractor->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_extractor->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                std::cout  << "=================================" << std::endl;
                std::cout << std::endl;
            }
            catch (itk::ExceptionObject& err)
            {
                std::cout << "ExceptionObject caught !" << std::endl;
                std::cout << err << std::endl;
                itkExceptionMacro("Error writing output");
            }
        }
    }

private:
    void BuildOutputImage() {
        m_inputImgReader->UpdateOutputInformation();

        float width = m_inputImgReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
        float height = m_inputImgReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
        float fMinWidth = width;
        float fMinHeight = height;
        if(m_outForcedWidth > 0 && width >= m_outForcedWidth) {
            fMinWidth = m_outForcedWidth;
        }
        if(m_outForcedHeight > 0 && height >= m_outForcedHeight) {
            fMinHeight = m_outForcedHeight;
        }

        m_extractor = ExtractROIFilterType::New();
        m_extractor->SetInput( m_inputImgReader->GetOutput() );
        m_extractor->SetSizeX(fMinWidth);
        m_extractor->SetSizeY(fMinHeight);
        m_extractor->UpdateOutputInformation();
    }

    typename ImageSource::Pointer m_inputImgReader;

    typename ExtractROIFilterType::Pointer m_extractor;
    std::string m_outputFileName;
    // during resampling at higher resolutions it might be needed to return with a specified dimension
    long m_outForcedWidth;
    long m_outForcedHeight;
};
#endif // CUTTINGIMAGEHANDLER_H
