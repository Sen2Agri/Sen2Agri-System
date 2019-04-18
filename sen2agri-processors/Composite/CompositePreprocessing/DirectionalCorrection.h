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

#ifndef DIRECTIONAL_CORRECTION_H
#define DIRECTIONAL_CORRECTION_H

#include "otbWrapperTypes.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "DirectionalCorrectionFunctor.h"
#include "MetadataHelperFactory.h"
#include "ResamplingBandExtractor.h"
#include "MetadataHelper.h"


class DirectionalCorrection
{
public:
    typedef float                                   PixelType;
    typedef short                                   PixelShortType;
    typedef otb::Wrapper::FloatVectorImageType                    InputImageType1;
    typedef otb::Image<PixelType, 2>                              InputImageType2;
    typedef otb::Wrapper::FloatImageType                          InternalBandImageType;
    typedef otb::Wrapper::Int16VectorImageType                    OutImageType;

    typedef otb::ImageList<InternalBandImageType>  ImageListType;
    typedef otb::ImageListToVectorImageFilter<ImageListType, InputImageType1 >    ListConcatenerFilterType;

    typedef DirectionalCorrectionFunctor <InputImageType1::PixelType,
                                    OutImageType::PixelType>                DirectionalCorrectionFunctorType;
    typedef itk::UnaryFunctorImageFilter< InputImageType1,
                                          OutImageType,
                                          DirectionalCorrectionFunctorType >      FunctorFilterType;

    typedef otb::ImageFileReader<InputImageType1> ReaderType;

public:
    DirectionalCorrection();

    void Init(int res, std::string &xml, std::string &scatcoef, InputImageType2::Pointer &cldImg,
              InputImageType2::Pointer &watImg, InputImageType2::Pointer &snowImg,
              InputImageType1::Pointer &angles, InputImageType2::Pointer &ndvi);

    void DoExecute();
    const char * GetNameOfClass() { return "DirectionalCorrection"; }

    OutImageType::Pointer GetCorrectedImg();

private:
    int extractBandsFromImage(InputImageType1::Pointer & imageType);
    std::vector<ScaterringFunctionCoefficients> loadScatteringFunctionCoeffs(std::string &strFileName);
    std::string trim(std::string const& str);

private:
    int                                     m_nRes;
    std::string                             m_strXml;
    std::string                             m_strScatCoeffs;

    InputImageType1::Pointer                m_L2AIn;
    InputImageType1::Pointer                m_AnglesImg;
    InputImageType2::Pointer                m_NdviImg, m_CSM, m_WM, m_SM;
    ImageListType::Pointer                  m_ImageList;
    ListConcatenerFilterType::Pointer       m_Concat;
    FunctorFilterType::Pointer              m_DirectionalCorrectionFunctor;
    DirectionalCorrectionFunctorType        m_Functor;

    ReaderType::Pointer                     m_inputImageReader;
    ResamplingBandExtractor<float>         m_ResampledBandsExtractor;
    std::unique_ptr<MetadataHelper<float>>  m_pHelper;
};

#endif // DIRECTIONAL_CORRECTION_H
