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
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

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
#include <vector>
#include "UpdateSynthesisFunctor_2.h"
#include "MetadataHelperFactory.h"
#include "BandsCfgMappingParser.h"
#include "ResamplingBandExtractor.h"
#include "GenericRSImageResampler.h"
#include "itkMetaDataDictionary.h"

namespace otb
{

namespace Wrapper
{
class UpdateSynthesis : public Application
{
public:
    typedef UpdateSynthesis Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(UpdateSynthesis, otb::Application)

    typedef float                                   PixelType;
    typedef short                                   PixelShortType;
    typedef FloatVectorImageType                    InputImageType;
    typedef FloatImageType                          InternalBandImageType;
    typedef Int16VectorImageType                    OutImageType;

    typedef otb::ImageFileReader<InputImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                                ImageReaderListType;

    typedef otb::ImageList<InternalBandImageType>  ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType, InputImageType >    ListConcatenerFilterType;
    typedef MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                         InternalBandImageType::PixelType>         ExtractROIFilterType;
    typedef ObjectList<ExtractROIFilterType>                                ExtractROIFilterListType;

    typedef UpdateSynthesisFunctor <InputImageType::PixelType,
                                    OutImageType::PixelType>                UpdateSynthesisFunctorType;
    typedef itk::UnaryFunctorImageFilter< InputImageType,
                                          OutImageType,
                                          UpdateSynthesisFunctorType >      FunctorFilterType;

private:

    void DoInit()
    {
        SetName("UpdateSynthesis");
        SetDescription("Update synthesis using the recurrent expression of the weighted average.");

        SetDocName("UpdateSynthesis");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");
        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputImage, "in", "L2A input product");
        AddParameter(ParameterType_Int, "res", "Input current L2A XML");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");
        AddParameter(ParameterType_InputFilename, "xml", "Input general L2A XML");
        AddParameter(ParameterType_InputFilename, "bmap", "Master to secondary bands mapping");
        AddParameter(ParameterType_InputImage, "csm", "Cloud-Shadow Mask");
        AddParameter(ParameterType_InputImage, "wm", "Water Mask");
        AddParameter(ParameterType_InputImage, "sm", "Snow Mask");
        AddParameter(ParameterType_InputImage, "wl2a", "Weights of L2A product for date N");

        AddParameter(ParameterType_InputImage, "prevl3aw", "Previous l3a product weights");
        MandatoryOff("prevl3aw");
        AddParameter(ParameterType_InputImage, "prevl3ad", "Previous l3a product dates");
        MandatoryOff("prevl3ad");
        AddParameter(ParameterType_InputImage, "prevl3ar", "Previous l3a product reflectances");
        MandatoryOff("prevl3ar");
        AddParameter(ParameterType_InputImage, "prevl3af", "Previous l3a product flags");
        MandatoryOff("prevl3af");

        AddParameter(ParameterType_OutputImage, "out", "Out image containing the number of bands of the master product, for the given resolution");

        m_ImageList = ImageListType::New();
        m_Concat = ListConcatenerFilterType::New();
        m_ExtractorList = ExtractROIFilterListType::New();
        m_ImageReaderList = ImageReaderListType::New();
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    // The algorithm consists in a applying a formula for computing the NDVI for each pixel,
    // using BandMathFilter
    void DoExecute()
    {
        int resolution = GetParameterInt("res");
        std::string inXml = GetParameterAsString("xml");
        std::string strBandsMappingFileName = GetParameterAsString("bmap");
        m_L2AIn = GetParameterFloatVectorImage("in");
        m_CSM = GetParameterFloatVectorImage("csm");
        m_WM = GetParameterFloatVectorImage("wm");
        m_SM = GetParameterFloatVectorImage("sm");
        m_WeightsL2A = GetParameterFloatVectorImage("wl2a");

        m_L2AIn->UpdateOutputInformation();
        m_CSM->UpdateOutputInformation();
        m_WM->UpdateOutputInformation();
        m_SM->UpdateOutputInformation();
        m_WeightsL2A->UpdateOutputInformation();

        if(resolution <= 0) {
            resolution = m_WeightsL2A->GetSpacing()[0];
        }


        InputImageType::SpacingType spacingL2AIn = m_L2AIn->GetSpacing();
        InputImageType::PointType originL2AIn = m_WeightsL2A->GetOrigin();
        InputImageType::SpacingType spacingCSM = m_CSM->GetSpacing();
        InputImageType::PointType originCSM = m_WeightsL2A->GetOrigin();
        InputImageType::SpacingType spacingWM = m_WM->GetSpacing();
        InputImageType::PointType originWM = m_WeightsL2A->GetOrigin();
        InputImageType::SpacingType spacingSM = m_SM->GetSpacing();
        InputImageType::PointType originSM = m_WeightsL2A->GetOrigin();
        InputImageType::SpacingType spacingWeightsL2A = m_WeightsL2A->GetSpacing();
        InputImageType::PointType originWeightsL2A = m_WeightsL2A->GetOrigin();

        // After with gdalwarp cut for an L8 that is used with S2 the spacing and origin might be changed.
        // The reference is the weight clouds that is corrected
        if((spacingWeightsL2A[0] != spacingL2AIn[0]) || (spacingWeightsL2A[1] != spacingL2AIn[1]) ||
           (originWeightsL2A[0] != originL2AIn[0]) || (originWeightsL2A[1] != originL2AIn[1])) {
            float fMultiplicationFactor = ((float)spacingL2AIn[0])/spacingWeightsL2A[0];
            //force the origin and the resolution to the one from cloud image
            m_L2AIn = m_Resampler.getResampler(m_L2AIn, fMultiplicationFactor, originWeightsL2A)->GetOutput();
        }
        if((spacingWeightsL2A[0] != spacingCSM[0]) || (spacingWeightsL2A[1] != spacingCSM[1]) ||
           (originWeightsL2A[0] != originCSM[0]) || (originWeightsL2A[1] != originCSM[1])) {
            float fMultiplicationFactor = ((float)spacingCSM[0])/spacingWeightsL2A[0];
            //force the origin and the resolution to the one from cloud image
            m_CSM = m_Resampler.getResampler(m_CSM, fMultiplicationFactor, originWeightsL2A, Interpolator_NNeighbor)->GetOutput();
        }
        if((spacingWeightsL2A[0] != spacingWM[0]) || (spacingWeightsL2A[1] != spacingWM[1]) ||
           (originWeightsL2A[0] != originWM[0]) || (originWeightsL2A[1] != originWM[1])) {
            float fMultiplicationFactor = ((float)spacingWM[0])/spacingWeightsL2A[0];
            //force the origin and the resolution to the one from cloud image
            m_WM = m_Resampler.getResampler(m_WM, fMultiplicationFactor, originWeightsL2A, Interpolator_NNeighbor)->GetOutput();
        }
        if((spacingWeightsL2A[0] != spacingSM[0]) || (spacingWeightsL2A[1] != spacingSM[1]) ||
           (originWeightsL2A[0] != originSM[0]) || (originWeightsL2A[1] != originSM[1])) {
            float fMultiplicationFactor = ((float)spacingSM[0])/spacingWeightsL2A[0];
            //force the origin and the resolution to the one from cloud image
            m_SM = m_Resampler.getResampler(m_SM, fMultiplicationFactor, originWeightsL2A, Interpolator_NNeighbor)->GetOutput();
        }

        auto szL2A = m_L2AIn->GetLargestPossibleRegion().GetSize();
        int nL2AWidth = szL2A[0];
        int nL2AHeight = szL2A[1];


        int nL3AWidth = -1;
        int nL3AHeight = -1;
        bool l3aExist = false;
        if(HasValue("prevl3aw") && HasValue("prevl3ad") &&
           HasValue("prevl3ar") && HasValue("prevl3af")) {
            l3aExist = true;
            m_PrevL3AWeight = GetParameterFloatVectorImage("prevl3aw");
            m_PrevL3AAvgDate = GetParameterFloatVectorImage("prevl3ad");
            m_PrevL3ARefl = GetParameterFloatVectorImage("prevl3ar");
            m_PrevL3AFlags = GetParameterFloatVectorImage("prevl3af");
            m_PrevL3AFlags->UpdateOutputInformation();
            auto szL3A = m_PrevL3AFlags->GetLargestPossibleRegion().GetSize();
            nL3AWidth = szL3A[0];
            nL3AHeight = szL3A[1];
        }

        m_bandsCfgMappingParser.ParseFile(strBandsMappingFileName);
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();

        int nDesiredWidth = -1;
        int nDesiredHeight = -1;
        // it is enough to check only one dimension
        if(nL3AWidth != -1) {
            if((nL3AWidth != nL2AWidth) || (nL3AHeight != nL2AHeight)) {
                nDesiredWidth = std::max(nL3AWidth, nL2AWidth);
                nDesiredHeight = std::max(nL3AHeight, nL2AHeight);
            }
        }

        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<float>(inXml);
        const std::string &missionName = m_pHelper->GetMissionName();
        int nExtractedBandsNo = 0;
        // create an array of bands presences with the same size as the master band size
        std::vector<int> bandsPresenceVect = bandsMappingCfg.GetBandsPresence(resolution, missionName, nExtractedBandsNo);

        // Using these indexes as they are extracted is not right
        // These indexes are from the current product and they should be translated to bands presence array indexes
        const std::string &blueBandName = m_pHelper->GetBlueBandName();
        // here we compute the relative indexes according to the presence array for the red and blue band
        int nRelBlueBandIdx = bandsMappingCfg.GetIndexInPresenceArray(resolution, missionName, blueBandName);
        //std::string masterMission = bandsMappingCfg.GetMasterMissionName();
        //std::vector<int> vectIdxs = bandsMappingCfg.GetAbsoluteBandIndexes(resolution, missionName);
        // Here we have a raster that already has the extracted bands configured in the file.
        // We do not get the bands from the product anymore but from this file (in file)
        InternalBandImageType::Pointer l2aBandImg;
        int nAddedBands = 0;
        for(unsigned int i = 0; i<bandsPresenceVect.size(); i++) {
            int nRelBandIdx = bandsPresenceVect[i];
            if(nRelBandIdx >= 0) {
                l2aBandImg = m_ResampledBandsExtractor.ExtractImgResampledBand(m_L2AIn, nRelBandIdx+1,
                                                         Interpolator_Linear, resolution, resolution, nDesiredWidth, nDesiredHeight);
                m_ImageList->PushBack(l2aBandImg);
                nAddedBands++;
            }
        }
        // normally, this should not happen
        if(nAddedBands != nExtractedBandsNo) {
            itkExceptionMacro("Inconsistent valid bands extracted from bands mapping");
        }
        // if the nRelBlueBandIdx is -1 resample and add it to the m_imageList from the 10m raster
        // NOTE: this happens only for Sentinel2
        bool bHasAppendedPrevL2ABlueBand = false;
        if(nRelBlueBandIdx == -1) {
            InputImageType::Pointer L2A10MResImg =  m_pHelper->GetImage({blueBandName}, NULL, 10);
            L2A10MResImg->UpdateOutputInformation();

            // extract the band without resampling it
            l2aBandImg = m_ResampledBandsExtractor.ExtractImgResampledBand(L2A10MResImg, 1,
                                                     Interpolator_Linear);
            l2aBandImg->UpdateOutputInformation();
            // check if we need to reproject this image if it is the case
            const std::string &bandProjRef = l2aBandImg->GetProjectionRef();
            // get the input image projection
            const std::string &inImgProjRef = m_L2AIn->GetProjectionRef();
            int l2aBandImgRes = l2aBandImg->GetSpacing()[0];
            const float scale = (float)resolution / l2aBandImgRes;

            InputImageType::PointType origin = m_L2AIn->GetOrigin();
            if(bandProjRef != inImgProjRef) {
                // no need to resample as already done before
                l2aBandImg = m_GenericRSImageResampler.getResampler(l2aBandImg, scale, nDesiredWidth,
                      nDesiredHeight, origin, Interpolator_Linear)
                      ->GetOutput();
            } else {
                // use the streaming resampler
                l2aBandImg = m_ImageResampler.getResampler(l2aBandImg, scale, nDesiredWidth,
                      nDesiredHeight, origin, Interpolator_Linear)
                      ->GetOutput();
            }
            l2aBandImg->UpdateOutputInformation();

            m_ImageList->PushBack(l2aBandImg);
            nRelBlueBandIdx = nExtractedBandsNo++;
            // add the added blue band also in the bands presence array
            bandsPresenceVect.push_back(nRelBlueBandIdx);
            bHasAppendedPrevL2ABlueBand = true;
        }

        //m_ResampledBandsExtractor.ExtractAllResampledBands(m_L2AIn, m_ImageList);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_CSM, m_ImageList, Interpolator_NNeighbor, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_WM, m_ImageList, Interpolator_NNeighbor, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_SM, m_ImageList, Interpolator_NNeighbor, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_WeightsL2A, m_ImageList, Interpolator_Linear, resolution, resolution, nDesiredWidth, nDesiredHeight);

        int nNbL3ABands = 0;
        if(HasValue("prevl3aw") && HasValue("prevl3ad") &&
           HasValue("prevl3ar") && HasValue("prevl3af")) {
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AWeight, m_ImageList, Interpolator_Linear, resolution, resolution, nDesiredWidth, nDesiredHeight);
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AAvgDate, m_ImageList, Interpolator_Linear, resolution, resolution, nDesiredWidth, nDesiredHeight);
            int l3bReflBandsNo = m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3ARefl, m_ImageList, Interpolator_Linear, resolution, resolution, nDesiredWidth, nDesiredHeight);
            nNbL3ABands += l3bReflBandsNo;
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AFlags, m_ImageList, Interpolator_NNeighbor, resolution, resolution, nDesiredWidth, nDesiredHeight);
        }

        m_Concat->SetInput(m_ImageList);

        int productDate = m_pHelper->GetAcquisitionDateAsDoy();
        m_Functor.Initialize(bandsPresenceVect, nExtractedBandsNo, nRelBlueBandIdx, bHasAppendedPrevL2ABlueBand, l3aExist,
                             productDate, m_pHelper->GetReflectanceQuantificationValue());
        m_UpdateSynthesisFunctor = FunctorFilterType::New();
        m_UpdateSynthesisFunctor->SetFunctor(m_Functor);
        m_UpdateSynthesisFunctor->SetInput(m_Concat->GetOutput());

        m_UpdateSynthesisFunctor->UpdateOutputInformation();
        int nbComponents = m_Functor.GetNbOfOutputComponents();
        m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nbComponents);

        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", m_UpdateSynthesisFunctor->GetOutput());
        //splitOutputs();

        return;
    }

    // get a reader from the file path
    ImageReaderType::Pointer getReader(const std::string& filePath) {
        ImageReaderType::Pointer reader = ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        return reader;
    }

/*
    typedef otb::ImageList<otb::ImageList<otb::VectorImage<short, 2> > >          InternalImageListType;
    typedef otb::Image<short, 2>                                                OutImageType1;
    typedef otb::ImageList<OutImageType1>                                       OutputImageListType;
    typedef otb::VectorImageToImageListFilter<OutImageType, OutputImageListType>   VectorImageToImageListType;


    typedef otb::ImageList<OutImageType1>  ImgListType;
    typedef otb::VectorImage<short, 2>                                        ImageType;
    typedef otb::ImageListToVectorImageFilter<ImgListType, ImageType>       ImageListToVectorImageFilterType;


    void splitOutputs() {
        weightList = ImgListType::New();
        reflectancesList = ImgListType::New();
        datesList = ImgListType::New();
        flagsList = ImgListType::New();
        allList = ImgListType::New();

        std::ostream & objOstream = std::cout;
        m_imgSplit = VectorImageToImageListType::New();
        m_functorOutput = m_UpdateSynthesisFunctor->GetOutput();
        m_functorOutput->UpdateOutputInformation();
        m_UpdateSynthesisFunctor->InPlaceOn();
        m_functorOutput->Print(objOstream);
        m_imgSplit->SetInput(m_functorOutput);
        m_imgSplit->UpdateOutputInformation();
        m_imgSplit->GetOutput()->UpdateOutputInformation();
        m_imgSplit->Print(objOstream);
        std::cout << m_imgSplit->GetNumberOfOutputs() << std::endl;

        int nL3AReflBandsNo = m_Functor.GetNbOfL3AReflectanceBands();
        int nCurBand = 0;
        for(int i = 0; i<nL3AReflBandsNo; i++, nCurBand++) {
            weightList->PushBack(m_imgSplit->GetOutput()->GetNthElement(nCurBand));
        }

        datesList->PushBack(m_imgSplit->GetOutput()->GetNthElement(nCurBand++));
        for(int i = 0; i<nL3AReflBandsNo; i++, nCurBand++) {
            reflectancesList->PushBack(m_imgSplit->GetOutput()->GetNthElement(nCurBand));
        }
        flagsList->PushBack(m_imgSplit->GetOutput()->GetNthElement(nCurBand++));

        m_weightsConcat = ImageListToVectorImageFilterType::New();
        m_weightsConcat->SetInput(weightList);
        SetParameterOutputImagePixelType("outweights", ImagePixelType_int16);
        SetParameterOutputImage("outweights", m_weightsConcat->GetOutput());

        m_reflsConcat = ImageListToVectorImageFilterType::New();
        m_reflsConcat->SetInput(reflectancesList);
        m_reflsConcat->GetOutput()->CopyInformation(m_imgSplit->GetOutput()->GetNthElement(0));
        //m_reflsConcat->GetOutput()->SetNumberOfComponentsPerPixel(m_imgSplit->GetOutput()->Size());
        //m_reflsConcat->GetOutput()->SetLargestPossibleRegion(m_imgSplit->GetOutput()->GetNthElement(0)->GetLargestPossibleRegion());

        SetParameterOutputImagePixelType("outrefls", ImagePixelType_int16);
        SetParameterOutputImage("outrefls", m_reflsConcat->GetOutput());

        //SetParameterOutputImagePixelType("outflags", ImagePixelType_int16);
        //SetParameterOutputImage("outflags", m_allConcat->GetOutput());

        //SetParameterOutputImagePixelType("outdates", ImagePixelType_int16);
        //SetParameterOutputImage("outdates", m_allConcat->GetOutput());

    }
*/
    InputImageType::Pointer             m_L2AIn;
    InputImageType::Pointer             m_CSM, m_WM, m_SM, m_WeightsL2A;
    InputImageType::Pointer             m_PrevL3AWeight, m_PrevL3AAvgDate, m_PrevL3ARefl, m_PrevL3AFlags;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_UpdateSynthesisFunctor;
    UpdateSynthesisFunctorType          m_Functor;

    ImageReaderListType::Pointer        m_ImageReaderList;

    BandsCfgMappingParser m_bandsCfgMappingParser;
    ResamplingBandExtractor<float> m_ResampledBandsExtractor;
    ImageResampler<InputImageType, InputImageType> m_Resampler;

    GenericRSImageResampler<InternalBandImageType, InternalBandImageType>  m_GenericRSImageResampler;
    ImageResampler<InternalBandImageType, InternalBandImageType>  m_ImageResampler;

    std::unique_ptr<MetadataHelper<float>> m_pHelper;
/*
    VectorImageToImageListType::Pointer       m_imgSplit;
    ImageListToVectorImageFilterType::Pointer m_allConcat;
    ImageListToVectorImageFilterType::Pointer m_weightsConcat;
    ImageListToVectorImageFilterType::Pointer m_reflsConcat;

    ImgListType::Pointer allList;
    ImgListType::Pointer weightList;
    ImgListType::Pointer reflectancesList;
    ImgListType::Pointer datesList;
    ImgListType::Pointer flagsList;

    OutImageType::Pointer m_functorOutput;
*/
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::UpdateSynthesis)


