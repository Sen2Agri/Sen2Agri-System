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
#include "UpdateSynthesisFunctor.h"
#include "MetadataHelperFactory.h"

namespace otb
{

namespace Wrapper
{

template< class TPixel>
class CustomFunctor
{
public:
  CustomFunctor() {}
  ~CustomFunctor() {}
  inline void SetTest(int x) {}
  /*
  bool operator!=(const CustomFunctor &) const
  {
    return false;
  }
  bool operator==(const CustomFunctor & other) const
  {
    return !( *this != other );
  }
  */
  inline TPixel operator()(const TPixel & A) const
  {
     Int16VectorImageType::PixelType var(2);
     var[0] = A[1];
     var[1] = A[2];
    return var;
  }
};
class UpdateSynthesis : public Application
{
public:
    typedef enum flagVal {
        land,
        cloud,
        shadow,
        snow,
        water
    } FLAG_VALUE;

    typedef UpdateSynthesis Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(UpdateSynthesis, otb::Application)

    typedef float                                   PixelType;
    typedef short                                   PixelShortType;

    typedef otb::ImageList<FloatImageType>  ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType,
                                         FloatVectorImageType >                   ListConcatenerFilterType;
    typedef MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                         FloatImageType::PixelType>               ExtractROIFilterType;
    typedef ObjectList<ExtractROIFilterType>                                      ExtractROIFilterListType;

    typedef UpdateSynthesisFunctor <FloatVectorImageType::PixelType, FloatVectorImageType::PixelType> UpdateSynthesisFunctorType;
    typedef itk::UnaryFunctorImageFilter< FloatVectorImageType, FloatVectorImageType,
                              UpdateSynthesisFunctorType > FunctorFilterType;

    /*
    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType, Int16VectorImageType,
                              CustomFunctor<Int16VectorImageType::PixelType> > BinaryFilterType;

    typedef itk::UnaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType,
                              CustomFunctor<Int16VectorImageType::PixelType> > FilterType__;
    */

private:

    void DoInit()
    {
        SetName("UpdateSynthesis");
        SetDescription("Update synthesis using the recurrent expression of the weighted average.");

        SetDocName("UpdateSynthesis");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");
        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputImage, "in", "L2A input product");
        AddParameter(ParameterType_Int, "res", "Input current L2A XML");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");
        AddParameter(ParameterType_InputFilename, "xml", "Input general L2A XML");
        AddParameter(ParameterType_InputImage, "csm", "Cloud-Shadow Mask");
        AddParameter(ParameterType_InputImage, "wm", "Water Mask");
        AddParameter(ParameterType_InputImage, "sm", "Snow Mask");
        AddParameter(ParameterType_InputImage, "wl2a", "Weights of L2A product for date N");

        //not mandatory
        //TODO: see if these parameters are optional or not. In this moment is set this condition
        // in order to be possible to test
        MandatoryOff("wm");
        MandatoryOff("sm");

        AddParameter(ParameterType_InputImage, "prevl3a", "Previous l3a product");
        MandatoryOff("prevl3a");
        /*
        AddParameter(ParameterType_InputImage, "prevw", "Weight for each pixel obtained so far");
        MandatoryOff("prevw");
        AddParameter(ParameterType_InputImage, "wavgdate", "Weighted average date for L3A product so far");
        MandatoryOff("wavgdate");
        AddParameter(ParameterType_InputImage, "wavgref", "Weighted average reflectance value so far, for each pixel and each spectral band");
        MandatoryOff("wavgref");
        AddParameter(ParameterType_InputImage, "pixstat", "Status of each L3A pixel: cloud, water, snow");
        MandatoryOff("pixstat");
        */
        // out rasters for L3A product
        /*
        AddParameter(ParameterType_OutputImage, "outw", "Out weight counter for each pixel and for each band");
        AddParameter(ParameterType_OutputImage, "outdate", "Out weighted average date for L3A product so far");
        AddParameter(ParameterType_OutputImage, "outro", "Out weighted average reflectance value so far for each pixel and each spectral band");
        AddParameter(ParameterType_OutputImage, "outstat", "Out status of each L3A pixel: cloud, water, snow");
        */

        //test only
        AddParameter(ParameterType_OutputImage, "out", "Out image containing 10 or 14 bands according to resolution");

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

        m_L2AIn = GetParameterFloatVectorImage("in");
        m_L2AIn->UpdateOutputInformation();
        if(resolution == -1) {
            resolution = m_L2AIn->GetSpacing()[0];
        }
        m_CSM = GetParameterFloatVectorImage("csm");

        //TODO: see if these parameters are optional or not. In this moment is set this condition
        // in order to be possible to test
        if(HasValue("wm"))
            m_WM = GetParameterFloatVectorImage("wm");
        else
            m_WM = m_CSM;

        //TODO: see if these parameters are optional or not. In this moment is set this condition
        // in order to be possible to test
        if(HasValue("sm"))
            m_SM = GetParameterFloatVectorImage("sm");
        else
            m_SM = m_CSM;

        m_WeightsL2A = GetParameterFloatVectorImage("wl2a");
        //Int16VectorImageType::Pointer inImage2 = GetParameterInt16VectorImage("in2");

        unsigned int j=0;
        for(j=0; j < m_L2AIn->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_L2AIn );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }
        // will the following have only one band, for sure? if yes, simply push_back each one of them
        for(j=0; j < m_CSM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_CSM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_WM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_WM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_SM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_SM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_WeightsL2A->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_WeightsL2A );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        bool l3aExist = false;
        if(HasValue("prevl3a")) {
            m_PrevL3A = GetParameterFloatVectorImage("prevl3a");
            l3aExist = true;
            for(j=0; j < m_PrevL3A->GetNumberOfComponentsPerPixel(); j++)
            {
                ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
                extractor->SetInput( m_PrevL3A );
                extractor->SetChannel( j+1 );
                extractor->UpdateOutputInformation();
                m_ExtractorList->PushBack( extractor );
                m_ImageList->PushBack( extractor->GetOutput() );
            }
        }

        m_Concat->SetInput(m_ImageList);

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml, resolution);
        SensorType sensorType;
        std::string missionName = pHelper->GetMissionName();
        otbAppLogINFO( << "Mission name" << missionName << std::endl );

        if(missionName.find(SENTINEL_MISSION_STR) != std::string::npos) {
            sensorType = SENSOR_S2;
        } else if (missionName.find(LANDSAT_MISSION_STR) != std::string::npos) {
            sensorType = SENSOR_LANDSAT8;
        } else if (missionName.find(SPOT4_MISSION_STR) != std::string::npos) {
            sensorType = SENSOR_SPOT4;
        }

        m_Functor.Initialize(sensorType, (resolution == 10 ? RES_10M : RES_20M), l3aExist);
        int productDate = pHelper->GetAcquisitionDateInDays();
        m_Functor.SetCurrentDate(productDate);
        m_Functor.SetReflectanceQuantificationValue(pHelper->GetReflectanceQuantificationValue());
        m_UpdateSynthesisFunctor = FunctorFilterType::New();
        m_UpdateSynthesisFunctor->SetFunctor(m_Functor);
        m_UpdateSynthesisFunctor->SetInput(m_Concat->GetOutput());
        m_UpdateSynthesisFunctor->UpdateOutputInformation();
        if(resolution == 10)
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(10);
        else
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(14);


        SetParameterOutputImage("out", m_UpdateSynthesisFunctor->GetOutput());

        return;
    }
    FloatVectorImageType::Pointer       m_L2AIn;
    FloatVectorImageType::Pointer       m_CSM, m_WM, m_SM, m_WeightsL2A;
    FloatVectorImageType::Pointer       m_PrevL3A;
    FloatVectorImageType::Pointer       m_PrevWeightPixel, m_WeightAvgDate, m_WeightAvgRef, m_PixelStat;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_UpdateSynthesisFunctor;
    UpdateSynthesisFunctorType          m_Functor;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::UpdateSynthesis)


