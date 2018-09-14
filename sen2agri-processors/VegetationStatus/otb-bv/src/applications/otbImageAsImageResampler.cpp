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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbStreamingResampleImageFilter.h"
#include "../../../../Common/OTBExtensions/otbGridResampleImageFilter.h"
#include "otbGenericRSResampleImageFilter.h"


namespace otb
{
namespace Wrapper
{
class ImageAsImageResampler : public Application
{
    typedef otb::VectorImage<float, 2>                                        ImageType;
    struct RefImgData {
        double                                m_imageWidth;
        double                                m_imageHeight;
        ImageType::PointType                  m_imageOrigin;
        std::string                           m_projection;
    };


    typedef typename itk::InterpolateImageFunction<ImageType, double>     InterpolateImageFunctionType;
    typename InterpolateImageFunctionType::Pointer interpolator;
    // Set the interpolator
    typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double>             NearestNeighborInterpolationType;
    typedef otb::BCOInterpolateImageFunction<ImageType, double>                         BicubicInterpolationType;

public:
    typedef ImageAsImageResampler Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(ImageAsImageResampler, otb::Application)

private:

    otb::ObjectList<itk::ProcessObject>::Pointer m_Filters = otb::ObjectList<itk::ProcessObject>::New();

    void DoInit()
    {
        SetName("ImageAsImageResampler");
        SetDescription("Resamples/reprojects an image according to another reference image.");

        SetDocName("ImageAsImageResampler");
        SetDocLongDescription("Resamples/reprojects an image according to another reference image.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_InputImage, "in", "The input image to be resampled");
        AddParameter(ParameterType_InputImage, "ref", "The reference image");
        AddParameter(ParameterType_Int, "ismask", "The input image is mask");
        AddParameter(ParameterType_OutputImage,  "out",   "The updated resampled/reprojected image");
    }

    void DoUpdateParameters()
    {
    }
    void DoExecute()
    {
        FloatVectorImageType::Pointer inputImage = GetParameterImage("in");
        FloatVectorImageType::Pointer refImage = GetParameterImage("ref");
        bool isMask = (GetParameterInt("ismask") != 0);

        // Extract the information from the reference image
        updateRequiredImageSize<FloatVectorImageType>(inputImage, refImage, m_refImgData);

        FloatVectorImageType::Pointer outImg = getResampledImage<FloatVectorImageType>(inputImage, refImage, m_refImgData, isMask);
        SetParameterOutputImage("out", outImg);
    }

    template<typename ImageType>
    typename ImageType::Pointer getResampledImage(const typename ImageType::Pointer& image, const typename ImageType::Pointer& refImg,
                                                  const RefImgData& td, bool isMask)
    {
        image->UpdateOutputInformation();
        refImg->UpdateOutputInformation();
        auto imageSize = image->GetLargestPossibleRegion().GetSize();
        float outRes = refImg->GetSpacing()[0];

        // Evaluate size
        typename ImageType::SizeType recomputedSize;
        recomputedSize[0] = td.m_imageWidth;
        recomputedSize[1] = td.m_imageHeight;

        std::string inputProjection = refImg->GetProjectionRef();
        bool sameProjection = inputProjection == td.m_projection;
        bool sameOrigin = refImg->GetOrigin() == td.m_imageOrigin;
        if (imageSize == recomputedSize && sameOrigin && sameProjection)
        {
            return image;
        }

        // Evaluate spacing
        typename ImageType::SpacingType outputSpacing;
        outputSpacing[0] = outRes;
        outputSpacing[1] = -outRes;

        float edgeValue;
        if (isMask)
        {
            interpolator = NearestNeighborInterpolationType::New();
            edgeValue = 1;
        }
        else
        {
            interpolator = BicubicInterpolationType::New();
            edgeValue = -10000;
        }

        typedef typename ImageType::PixelType PixelType;

        PixelType edgePixel;
        itk::NumericTraits<PixelType>::SetLength(edgePixel, image->GetNumberOfComponentsPerPixel());
        edgePixel = edgeValue * itk::NumericTraits<PixelType>::OneValue(edgePixel);

        typename ImageType::Pointer output;
        if (sameProjection)
        {
            if (sameOrigin)
            {
                typedef otb::GridResampleImageFilter<ImageType, ImageType, double>  ResampleFilterType;

                typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
                m_Filters->PushBack(resampler);

                resampler->SetInput(image);
                resampler->SetInterpolator(interpolator);
                resampler->SetOutputParametersFromImage(image);
                resampler->SetOutputSpacing(outputSpacing);
                resampler->SetOutputOrigin(td.m_imageOrigin);
                resampler->SetOutputSize(recomputedSize);
                resampler->SetEdgePaddingValue(edgePixel);
                resampler->SetCheckOutputBounds(false);

                output = resampler->GetOutput();
            }
            else
            {
                typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>  ResampleFilterType;

                typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
                m_Filters->PushBack(resampler);

                resampler->SetInput(image);
                resampler->SetInterpolator(interpolator);
                resampler->SetOutputParametersFromImage(image);
                resampler->SetOutputSpacing(outputSpacing);
                resampler->SetOutputOrigin(td.m_imageOrigin);
                resampler->SetOutputSize(recomputedSize);
                resampler->SetEdgePaddingValue(edgePixel);

                output = resampler->GetOutput();
            }
        }
        else
        {
            typedef otb::GenericRSResampleImageFilter<ImageType, ImageType>     ReprojectResampleFilterType;

            typename ReprojectResampleFilterType::Pointer resampler = ReprojectResampleFilterType::New();
            m_Filters->PushBack(resampler);

            resampler->SetInput(image);
            resampler->SetInputProjectionRef(inputProjection);
            resampler->SetOutputProjectionRef(td.m_projection);
            resampler->SetInputKeywordList(image->GetImageKeywordlist());
            resampler->SetInterpolator(interpolator);
            resampler->SetOutputSpacing(outputSpacing);
            resampler->SetDisplacementFieldSpacing(outputSpacing * 20);
            resampler->SetOutputOrigin(td.m_imageOrigin);
            resampler->SetOutputSize(recomputedSize);
            resampler->SetEdgePaddingValue(edgePixel);

            output = resampler->GetOutput();
        }
        return output;
    }

    template<typename ImageType>
    void updateRequiredImageSize(const typename ImageType::Pointer& image, const typename ImageType::Pointer& refImg, RefImgData& td) {
        image->UpdateOutputInformation();
        refImg->UpdateOutputInformation();
        float curRes = image->GetSpacing()[0];
        float outRes = refImg->GetSpacing()[0];

        const float scale = (float)outRes / curRes;
        td.m_imageWidth = image->GetLargestPossibleRegion().GetSize()[0] / scale;
        td.m_imageHeight = image->GetLargestPossibleRegion().GetSize()[1] / scale;

        auto origin = image->GetOrigin();
        auto spacing = image->GetSpacing();
        td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
        td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

        td.m_projection = image->GetProjectionRef();
    }

    RefImgData m_refImgData;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ImageAsImageResampler)


