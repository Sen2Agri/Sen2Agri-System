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
        SetName("UpdateSynthesis2");
        SetDescription("Update synthesis using the recurrent expression of the weighted average.");

        SetDocName("UpdateSynthesis2");
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
        m_L2AIn->UpdateOutputInformation();
        if(resolution <= 0) {
            resolution = m_L2AIn->GetSpacing()[0];
        }
        m_CSM = GetParameterFloatVectorImage("csm");
        m_WM = GetParameterFloatVectorImage("wm");
        m_SM = GetParameterFloatVectorImage("sm");
        m_WeightsL2A = GetParameterFloatVectorImage("wl2a");

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

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml, resolution);
        std::string missionName = pHelper->GetMissionName();
        int nRedBandIdx = pHelper->GetRedBandIndex();
        int nBlueBandIdx = pHelper->GetBlueBandIndex();

        m_bandsCfgMappingParser.ParseFile(strBandsMappingFileName);
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();

        // TODO: Maybe the mission name should be considered
        //std::string masterMissionName = bandsMappingCfg.GetMasterMissionName();

        int nDesiredWidth = -1;
        int nDesiredHeight = -1;
        // it is enough to check only one dimension
        if(nL3AWidth != -1) {
            if((nL3AWidth != nL2AWidth) || (nL3AHeight != nL2AHeight)) {
                nDesiredWidth = std::max(nL3AWidth, nL2AWidth);
                nDesiredHeight = std::max(nL3AHeight, nL2AHeight);
            }
        }

        int nExtractedBandsNo = 0;
        // create an array of bands presences with the same size as the master band size
        std::vector<int> bandsPresenceVect = bandsMappingCfg.GetBandsPresence(resolution, missionName, nExtractedBandsNo);
        //std::vector<int> vectIdxs = bandsMappingCfg.GetAbsoluteBandIndexes(resolution, missionName);
        // Here we have a raster that already has the extracted bands configured in the file.
        // We do not get the bands from the product anymore but from this file (in file)
        for(unsigned int i = 0; i<bandsPresenceVect.size(); i++) {
            //int nRelBandIdx = pHelper->GetRelativeBandIndex(vectIdxs[i]);
            int nRelBandIdx = bandsPresenceVect[i];
            if(nRelBandIdx >= 0) {
                InternalBandImageType::Pointer l2aBandImg = m_ResampledBandsExtractor.ExtractResampledBand(m_L2AIn, nRelBandIdx+1,
                                                                            resolution, resolution, nDesiredWidth, nDesiredHeight);
                m_ImageList->PushBack(l2aBandImg);
            }
        }

        //m_ResampledBandsExtractor.ExtractAllResampledBands(m_L2AIn, m_ImageList);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_CSM, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_WM, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_SM, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
        m_ResampledBandsExtractor.ExtractAllResampledBands(m_WeightsL2A, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);

        int nNbL3ABands = 0;
        if(HasValue("prevl3aw") && HasValue("prevl3ad") &&
           HasValue("prevl3ar") && HasValue("prevl3af")) {
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AWeight, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AAvgDate, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3ARefl, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
            nNbL3ABands += m_ResampledBandsExtractor.ExtractAllResampledBands(m_PrevL3AFlags, m_ImageList, resolution, resolution, nDesiredWidth, nDesiredHeight);
        }

        m_Concat->SetInput(m_ImageList);

        int productDate = pHelper->GetAcquisitionDateAsDoy();
        m_Functor.Initialize(bandsPresenceVect, nExtractedBandsNo, nRedBandIdx, nBlueBandIdx, l3aExist,
                             productDate, pHelper->GetReflectanceQuantificationValue());
        m_UpdateSynthesisFunctor = FunctorFilterType::New();
        m_UpdateSynthesisFunctor->SetFunctor(m_Functor);
        m_UpdateSynthesisFunctor->SetInput(m_Concat->GetOutput());

        m_UpdateSynthesisFunctor->SetDirectionTolerance(5);
        m_UpdateSynthesisFunctor->SetCoordinateTolerance(5);

        m_UpdateSynthesisFunctor->UpdateOutputInformation();
        int nbComponents = m_Functor.GetNbOfOutputComponents();
        m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nbComponents);
        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", m_UpdateSynthesisFunctor->GetOutput());
        //splitOutputs();

        return;
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


    BandsCfgMappingParser m_bandsCfgMappingParser;
    ResamplingBandExtractor m_ResampledBandsExtractor;
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


