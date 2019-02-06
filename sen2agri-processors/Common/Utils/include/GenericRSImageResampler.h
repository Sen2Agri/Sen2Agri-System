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
 
#ifndef GENERIC_RS_IMAGE_RESAMPLER_H
#define GENERIC_RS_IMAGE_RESAMPLER_H

#include "otbWrapperTypes.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbGenericRSResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"

template <class TInput, class TOutput, class TInput2 = TInput>
class GenericRSImageResampler
{
public:
    /** Generic Remote Sensor Resampler */
    typedef otb::GenericRSResampleImageFilter<TInput, TOutput>                        ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                       ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<TOutput, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<TOutput, double>                      LinearInterpolationType;
    typedef otb::BCOInterpolateImageFunction<TOutput>                                 BCOInterpolationType;
    typedef itk::IdentityTransform<double, TOutput::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, TOutput::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

    typedef typename NearestNeighborInterpolationType::Pointer NearestNeighborInterpolationTypePtr;
    typedef typename LinearInterpolationType::Pointer LinearInterpolationTypePtr;
    typedef typename IdentityTransformType::Pointer IdentityTransformTypePtr;
    typedef typename TInput::Pointer ResamplerInputImgPtr;
    typedef typename TInput2::Pointer ResamplerInputImgPtr2;
    typedef typename TInput::PixelType ResamplerInputImgPixelType;
    typedef typename TInput::SpacingType ResamplerInputImgSpacingType;
    typedef typename ResampleFilterType::Pointer ResamplerPtr;
    typedef typename ResampleFilterType::SizeType ResamplerSizeType;
    typedef typename otb::ObjectList<ResampleFilterType>::Pointer   ResampleFilterListTypePtr;

public:
    const char * GetNameOfClass() { return "ImageResampler"; }

    GenericRSImageResampler()
    {
        m_ResamplersList = ResampleFilterListType::New();
        m_BCORadius = 2;
        m_fBCOAlpha = -0.5;
        m_NoDataValue = NO_DATA_VALUE;
    }

    void SetBicubicInterpolatorParameters(int BCORadius, float BCOAlpha = -0.5) {
        m_BCORadius = BCORadius;
        m_fBCOAlpha = BCOAlpha;
    }

    void SetOutputProjection(const std::string &projRef) {
        m_OutputProjectionRef = projRef;
    }

    void SetNoDataValue(int noDataVal) {
        m_NoDataValue = noDataVal;
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const OutputVectorType& scale,
                             int forcedWidth, int forcedHeight, typename TOutput::PointType origin,
                              Interpolator_Type interpolatorType=Interpolator_Linear) {
         ResamplerPtr resampler = ResampleFilterType::New();
         resampler->SetInput(image);
         std::string srcProjRef = image->GetProjectionRef();
         auto srcImgKeywordList = image->GetImageKeywordlist();

         // Set the output projection Ref
         resampler->SetInputProjectionRef(srcProjRef);
         resampler->SetInputKeywordList(srcImgKeywordList);
         resampler->SetOutputProjectionRef(m_OutputProjectionRef);


         // Set the interpolator
         switch ( interpolatorType )
         {
             case Interpolator_Linear:
             {
                 typename LinearInterpolationType::Pointer interpolatorPtr = LinearInterpolationType::New();
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
             case Interpolator_NNeighbor:
             {
                 typename NearestNeighborInterpolationType::Pointer interpolatorPtr = NearestNeighborInterpolationType::New();
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
             case Interpolator_BCO:
             {
                 typename BCOInterpolationType::Pointer interpolatorPtr = BCOInterpolationType::New();
                 interpolatorPtr->SetRadius(m_BCORadius);
                 interpolatorPtr->SetAlpha(m_fBCOAlpha);
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
         }

         // Evaluate spacing
         ResamplerInputImgSpacingType spacing = image->GetSpacing();
         ResamplerInputImgSpacingType OutputSpacing;
         OutputSpacing[0] = std::round(spacing[0] * scale[0]);
         OutputSpacing[1] = std::round(spacing[1] * scale[1]);
         if (OutputSpacing[0] == 0) {
             OutputSpacing[0] = spacing[0] * scale[0];
         }
         if (OutputSpacing[1] == 0) {
             OutputSpacing[1] = spacing[1] * scale[1];
         }

         resampler->SetOutputSpacing(OutputSpacing);
         //resampler->SetDisplacementFieldSpacing(OutputSpacing[0] * 10);
         resampler->SetOutputOrigin(origin);

         // Evaluate size
         ResamplerSizeType recomputedSize;
         if((forcedWidth != -1) && (forcedHeight != -1))
         {
             recomputedSize[0] = forcedWidth;
             recomputedSize[1] = forcedHeight;
         } else {
            recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
            recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];
         }

         resampler->SetOutputSize(recomputedSize);

         ResamplerInputImgPixelType defaultValue;
         itk::NumericTraits<ResamplerInputImgPixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
         if(interpolatorType != Interpolator_NNeighbor) {
             defaultValue = NO_DATA_VALUE;
         }
         resampler->SetEdgePaddingValue(defaultValue);

         m_ResamplersList->PushBack(resampler);
         return resampler;
    }

    ResamplerPtr getResampler(const ResamplerInputImgPtr& image, const ResamplerInputImgPtr2 &inOrtho,
                              Interpolator_Type interpolatorType=Interpolator_Linear) {
        typename TOutput::PointType orig = inOrtho->GetOrigin();
        typename TOutput::SpacingType spacing = inOrtho->GetSpacing();
        typename TOutput::SizeType size = inOrtho->GetLargestPossibleRegion().GetSize();
        m_OutputProjectionRef = inOrtho->GetProjectionRef();
        int outputssizex = size[0];
        int outputssizey = size[1];
        float outputsspacingx = spacing[0];
        float outputsspacingy = spacing[1];
        float outputsulx = orig[0] - 0.5 * spacing[0];
        float outputsuly = orig[1] - 0.5 * spacing[1];
        // Update lower right
//        float outputslrx = outputsulx + outputsspacingx * static_cast<double>(outputssizex);
//        float outputslry = outputsuly + outputsspacingy * static_cast<double>(outputssizey);

         ResamplerPtr resampler = ResampleFilterType::New();
         resampler->SetInput(image);
         std::string srcProjRef = image->GetProjectionRef();
         auto srcImgKeywordList = image->GetImageKeywordlist();

         // Set the output projection Ref
         resampler->SetInputProjectionRef(srcProjRef);
         resampler->SetInputKeywordList(srcImgKeywordList);
         resampler->SetOutputProjectionRef(m_OutputProjectionRef);


         // Set the interpolator
         switch ( interpolatorType )
         {
             case Interpolator_Linear:
             {
                 typename LinearInterpolationType::Pointer interpolatorPtr = LinearInterpolationType::New();
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
             case Interpolator_NNeighbor:
             {
                 typename NearestNeighborInterpolationType::Pointer interpolatorPtr = NearestNeighborInterpolationType::New();
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
             case Interpolator_BCO:
             {
                 typename BCOInterpolationType::Pointer interpolatorPtr = BCOInterpolationType::New();
                 interpolatorPtr->SetRadius(m_BCORadius);
                 interpolatorPtr->SetAlpha(m_fBCOAlpha);
                 resampler->SetInterpolator(interpolatorPtr);
             }
             break;
         }

         // Evaluate spacing
         ResamplerInputImgSpacingType OutputSpacing;
         OutputSpacing[0] = outputsspacingx;
         OutputSpacing[1] = outputsspacingy;
         resampler->SetOutputSpacing(OutputSpacing);

         ResamplerInputImgSpacingType gridSpacing;
         gridSpacing[0] = 240;
         gridSpacing[1] = -240;
         resampler->SetDisplacementFieldSpacing(gridSpacing);

         typename TOutput::PointType origin;
         origin[0] = outputsulx + 0.5 * outputsspacingx;
         origin[1] = outputsuly + 0.5 * outputsspacingy;
         resampler->SetOutputOrigin(origin);

         // Evaluate size
         ResamplerSizeType recomputedSize;
         recomputedSize[0] = outputssizex;
         recomputedSize[1] = outputssizey;
         resampler->SetOutputSize(recomputedSize);

         ResamplerInputImgPixelType defaultValue;
         itk::NumericTraits<ResamplerInputImgPixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
         if(interpolatorType != Interpolator_NNeighbor) {
             defaultValue = m_NoDataValue;
         }
         resampler->SetEdgePaddingValue(defaultValue);

         m_ResamplersList->PushBack(resampler);
         return resampler;
    }


private:
    ResampleFilterListTypePtr             m_ResamplersList;
    int     m_BCORadius;
    float   m_fBCOAlpha;
    std::string                     m_OutputProjectionRef;
    int                             m_NoDataValue;
};

#endif // GENERIC_RS_IMAGE_RESAMPLER_H

