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
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageListToVectorImageFilter.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"

namespace otb
{
namespace Wrapper
{
class ResampleAtS2Res : public Application
{
public:
    typedef ResampleAtS2Res Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(ResampleAtS2Res, otb::Application)

    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;
    typedef otb::ImageFileWriter<ImageType>            WriterType;

    typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                              InternalImageType::PixelType>     ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                               ExtractROIFilterListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;
    typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                         ImageType >                   ListConcatenerFilterType;

    typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                      LinearInterpolationType;
    typedef otb::BCOInterpolateImageFunction<InternalImageType>                                 BCOInterpolationType;
    typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;

    typedef ScalableTransformType::OutputVectorType     OutputVectorType;

private:

    void DoInit()
    {
        SetName("ResampleAtS2Res");
        SetDescription("Resample the corresponding bands from LANDSAT or SPOT to S2 resolution");

        SetDocName("ResampleAtS2Res");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");


        AddDocTag(Tags::Vector);
        m_ConcatenerRes10 = ListConcatenerFilterType::New();
        m_ConcatenerRes20 = ListConcatenerFilterType::New();
        m_ConcatenerResOrig = ListConcatenerFilterType::New();

        m_ImageListRes10 = InternalImageListType::New();
        m_ImageListRes20 = InternalImageListType::New();
        m_ImageListResOrig = InternalImageListType::New();

        m_ResamplersList = ResampleFilterListType::New();
        m_ExtractorList = ExtractROIFilterListType::New();
        m_ImageReaderList = ImageReaderListType::New();

        AddParameter(ParameterType_String, "xml", "General xml input file for L2A MACCS product");
        AddParameter(ParameterType_String, "spotmask", "Image with 3 bands with cloud, water and snow masks for SPOT only");
        MandatoryOff("spotmask");

        AddParameter(ParameterType_Int, "allinone", "Specifies if all bands should be resampled at 10m and 20m");
        SetDefaultParameterInt("allinone", 0);
        MandatoryOff("allinone");

        AddParameter(ParameterType_OutputImage, "outres10", "Out Image at 10m resolution");
        MandatoryOff("outres10");
        AddParameter(ParameterType_OutputImage, "outcmres10", "Out cloud mask image at 10m resolution");
        MandatoryOff("outcmres10");
        AddParameter(ParameterType_OutputImage, "outwmres10", "Out water mask image at 10m resolution");
        MandatoryOff("outwmres10");
        AddParameter(ParameterType_OutputImage, "outsmres10", "Out snow mask image at 10m resolution");
        MandatoryOff("outsmres10");
        AddParameter(ParameterType_OutputImage, "outaotres10", "Out snow mask image at 10m resolution");
        MandatoryOff("outaotres10");

        AddParameter(ParameterType_OutputImage, "outres20", "Out Image at 10m resolution");
        MandatoryOff("outres20");
        AddParameter(ParameterType_OutputImage, "outcmres20", "Out cloud mask image at 20m resolution");
        MandatoryOff("outcmres20");
        AddParameter(ParameterType_OutputImage, "outwmres20", "Out water mask image at 20m resolution");
        MandatoryOff("outwmres20");
        AddParameter(ParameterType_OutputImage, "outsmres20", "Out snow mask image at 20m resolution");
        MandatoryOff("outsmres20");
        AddParameter(ParameterType_OutputImage, "outaotres20", "Out snow mask image at 20m resolution");
        MandatoryOff("outaotres20");

        AddParameter(ParameterType_OutputImage, "outres", "Out Image at the original resolution");
        MandatoryOff("outres");
        AddParameter(ParameterType_OutputImage, "outcmres", "Out cloud mask image at the original  resolution");
        MandatoryOff("outcmres");
        AddParameter(ParameterType_OutputImage, "outwmres", "Out water mask image at the original  resolution");
        MandatoryOff("outwmres");
        AddParameter(ParameterType_OutputImage, "outsmres", "Out snow mask image at the original  resolution");
        MandatoryOff("outsmres");
        AddParameter(ParameterType_OutputImage, "outaotres", "Out snow mask image at the original  resolution");
        MandatoryOff("outaotres");

        SetDocExampleParameterValue("xml", "/path/to/L2Aproduct_maccs.xml");
        SetDocExampleParameterValue("spotmask", "/path/to/spotmasks.tif");
        SetDocExampleParameterValue("allinone", "1");
        SetDocExampleParameterValue("outres10", "/path/to/output_image10.tif");
        SetDocExampleParameterValue("outcmres10", "/path/to/output_image_cloud10.tif");
        SetDocExampleParameterValue("outwmres10", "/path/to/output_image10_water10.tif");
        SetDocExampleParameterValue("outsmres10", "/path/to/output_image10_snow10.tif");
        SetDocExampleParameterValue("outaotres10", "/path/to/output_image_aot10.tif");

        SetDocExampleParameterValue("outres20", "/path/to/output_image20.tif");
        SetDocExampleParameterValue("outcmres20", "/path/to/output_image_cloud20.tif");
        SetDocExampleParameterValue("outwmres20", "/path/to/output_image10_water20.tif");
        SetDocExampleParameterValue("outsmres20", "/path/to/output_image10_snow20.tif");
        SetDocExampleParameterValue("outaotres20", "/path/to/output_image_aot20.tif");

        SetDocExampleParameterValue("outres", "/path/to/output_image.tif");
        SetDocExampleParameterValue("outcmres", "/path/to/output_image_cloud.tif");
        SetDocExampleParameterValue("outwmres", "/path/to/output_image_water.tif");
        SetDocExampleParameterValue("outsmres", "/path/to/output_image_snow.tif");
        SetDocExampleParameterValue("outaotres", "/path/to/output_image_aot.tif");

    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        const std::string &tmp = GetParameterAsString("xml");
        m_DirName = dirname(tmp);

        bool allInOne = (GetParameterInt("allinone") != 0);

        auto maccsReader = itk::MACCSMetadataReader::New();
        if (auto m = maccsReader->ReadMetadata(GetParameterAsString("xml")))
        {
            ProcessLANDSAT8(m, allInOne);
        }
        else
        {
            auto spot4Reader = itk::SPOT4MetadataReader::New();
            if (auto m = spot4Reader->ReadMetadata(GetParameterAsString("xml")))
            {
                ProcessSPOT4(m, allInOne);
            }
        }
        m_ConcatenerRes10->SetInput( m_ImageListRes10 );
        m_ConcatenerRes20->SetInput( m_ImageListRes20 );
        m_ConcatenerResOrig->SetInput( m_ImageListResOrig );

        if(HasValue("outres10"))
            SetParameterOutputImage("outres10", m_ConcatenerRes10->GetOutput());

        if(HasValue("outcmres10"))
            SetParameterOutputImage("outcmres10", m_ImageCloudRes10.GetPointer());

        if(HasValue("outwmres10"))
            SetParameterOutputImage("outwmres10", m_ImageWaterRes10.GetPointer());

        if(HasValue("outsmres10"))
            SetParameterOutputImage("outsmres10", m_ImageSnowRes10.GetPointer());

        if(HasValue("outaotres10"))
            SetParameterOutputImage("outaotres10", m_ImageAotRes10.GetPointer());

        if(HasValue("outres20"))
            SetParameterOutputImage("outres20", m_ConcatenerRes20->GetOutput());

        if(HasValue("outcmres20"))
            SetParameterOutputImage("outcmres20", m_ImageCloudRes20.GetPointer());

        if(HasValue("outwmres20"))
            SetParameterOutputImage("outwmres20", m_ImageWaterRes20.GetPointer());

        if(HasValue("outsmres20"))
            SetParameterOutputImage("outsmres20", m_ImageSnowRes20.GetPointer());

        if(HasValue("outaotres20"))
            SetParameterOutputImage("outaotres20", m_ImageAotRes20.GetPointer());

        // outputs at the same resolution as the input
        if(HasValue("outres"))
            SetParameterOutputImage("outres", m_ConcatenerResOrig->GetOutput());

        if(HasValue("outcmres"))
            SetParameterOutputImage("outcmres", m_ImageCloudResOrig.GetPointer());

        if(HasValue("outwmres"))
            SetParameterOutputImage("outwmres", m_ImageWaterResOrig.GetPointer());

        if(HasValue("outsmres"))
            SetParameterOutputImage("outsmres", m_ImageSnowResOrig.GetPointer());

        if(HasValue("outaotres"))
            SetParameterOutputImage("outaotres", m_ImageAotResOrig.GetPointer());

    }


    bool ProcessSPOT4(const std::unique_ptr<SPOT4Metadata>& meta, bool allInOne)
    {
        if(meta->Radiometry.Bands.size() != 4) {
            itkExceptionMacro("Wrong number of bands for SPOT4: " + meta->Radiometry.Bands.size() );
            return false;
        }

        std::string imageFile = m_DirName + "/" + meta->Files.OrthoSurfCorrPente;


        ImageReaderType::Pointer reader = getReader(imageFile);
        reader->UpdateOutputInformation();
        int curRes = reader->GetOutput()->GetSpacing()[0];

        std::vector<std::string>::iterator it;
        int i = 0;
        ExtractROIFilterType::Pointer extractor;
        for (it = meta->Radiometry.Bands.begin(), i = 1; it != meta->Radiometry.Bands.end(); it++, i++)
        {
            extractor = ExtractROIFilterType::New();
            extractor->SetInput( reader->GetOutput() );
            extractor->SetChannel( i );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );

            m_ImageListResOrig->PushBack(extractor->GetOutput());
            if(allInOne) {
                m_ImageListRes10->PushBack(getResampledImage(curRes, 10, extractor, false));
                m_ImageListRes20->PushBack(getResampledImage(curRes, 20, extractor, false));
            } else {
                if((*it).compare("XS1") == 0 || (*it).compare("XS2") == 0 || (*it).compare("XS3") == 0) {
                    // resample to 10m
                    m_ImageListRes10->PushBack(getResampledImage(curRes, 10, extractor, false));
                }
                else {
                    if((*it).compare("SWIR") == 0) {
                        m_ImageListRes20->PushBack(getResampledImage(curRes, 20, extractor, false));
                    } else {
                        itkExceptionMacro("Wrong band name for SPOT4: " + (*it));
                        return false;
                    }
                }
            }
        }
        imageFile = m_DirName + "/" +meta->Files.MaskNua;

        if(!HasValue("spotmask")) {
            itkExceptionMacro("The mask file for SPOT was not provided, do set 'spotmask' flag ");
            return false;
        }
        ImageReaderType::Pointer spotMasks = getReader(GetParameterAsString("spotmask"));

        //Resample the cloud mask
        extractor = ExtractROIFilterType::New();
        extractor->SetInput( spotMasks->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageCloudRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageCloudRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageCloudResOrig = extractor->GetOutput();

        //Resample the water mask
        extractor = ExtractROIFilterType::New();
        extractor->SetInput( spotMasks->GetOutput() );
        extractor->SetChannel( 2 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageWaterRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageWaterRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageWaterResOrig = extractor->GetOutput();

        //Resample the snow mask
        extractor = ExtractROIFilterType::New();
        extractor->SetInput( spotMasks->GetOutput() );
        extractor->SetChannel( 3 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageSnowRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageSnowRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageSnowResOrig = extractor->GetOutput();

        // resample the AOT
        std::string aotFileName = getSpot4AotFileName(meta);
        otbAppLogINFO( << "AOT file name" << aotFileName << std::endl );
        ImageReaderType::Pointer aotImage = getReader(aotFileName);
        extractor = ExtractROIFilterType::New();
        extractor->SetInput( aotImage->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageAotRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageAotRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageAotResOrig = extractor->GetOutput();

        return true;

    }

    bool ProcessLANDSAT8(const std::unique_ptr<MACCSFileMetadata>& meta, bool allInOne)
    {
        if(meta->ImageInformation.Bands.size() != 8) {
            itkExceptionMacro("Wrong number of bands for LANDSAT: " + meta->ImageInformation.Bands.size() );
            return false;
        }
        std::string imageXMLFile("");
        std::string cloudXMLFile("");
        std::string waterXMLFile("");
        std::string snowXMLFile("");
        std::string aotXMLFile("");
        for(auto fileInf = meta->ProductOrganization.ImageFiles.begin(); fileInf != meta->ProductOrganization.ImageFiles.end(); fileInf++)
        {
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_FRE") == 0 && fileInf->FileLocation.size() > 0)
                imageXMLFile = m_DirName + "/" + fileInf->FileLocation;
            else
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_CLD") == 0 && fileInf->FileLocation.size() > 0)
                cloudXMLFile = m_DirName + "/" + fileInf->FileLocation;
            else
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_WAT") == 0 && fileInf->FileLocation.size() > 0)
                waterXMLFile = m_DirName + "/" + fileInf->FileLocation;
            else
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_SNW") == 0 && fileInf->FileLocation.size() > 0)
                snowXMLFile = m_DirName + "/" + fileInf->FileLocation;
        }

        for(auto fileInf = meta->ProductOrganization.AnnexFiles.begin(); fileInf != meta->ProductOrganization.AnnexFiles.end(); fileInf++)
        {
            if(fileInf->File.LogicalName.substr(fileInf->File.LogicalName.size() - 4, 4).compare("_ATB") == 0 && fileInf->File.FileLocation.size() > 0)
                aotXMLFile = m_DirName + "/" + fileInf->File.FileLocation;
        }
        int nAotBandIdx = 1;
        // For MACCS, AOT is set as the band 2 in the cf. ATB file
        for (auto band : meta->ImageInformation.Bands) {
            if (band.Name == "AOT") {
                nAotBandIdx = std::stoi(band.Id);
            }
        }

        if(imageXMLFile.empty()) //TODO add error msg
            return false;

        auto maccsImageReader = itk::MACCSMetadataReader::New();
        std::unique_ptr<MACCSFileMetadata> imageMeta = nullptr;

        if ((imageMeta = maccsImageReader->ReadMetadata(imageXMLFile)) == nullptr) //TODO add error msg
            return false;

        if(imageMeta->ImageInformation.Bands.size() != 8) //TODO add error msg
            return false;

        //create image filename
        std::string imageFile = imageXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer reader = getReader(imageFile);
        reader->UpdateOutputInformation();
        int curRes = reader->GetOutput()->GetSpacing()[0];

        std::vector<MACCSBand>::iterator it;
        int i = 0;
        ExtractROIFilterType::Pointer extractor;
        for (it = imageMeta->ImageInformation.Bands.begin(), i = 1; it != imageMeta->ImageInformation.Bands.end(); it++, i++)
        {
            extractor = ExtractROIFilterType::New();
            extractor->SetInput( reader->GetOutput() );
            extractor->SetChannel( i );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );

            m_ImageListResOrig->PushBack(extractor->GetOutput());
            if(allInOne) {
                // resample to 10m
                m_ImageListRes10->PushBack(getResampledImage(curRes, 10, extractor, false));

                // resample to 20m
                m_ImageListRes20->PushBack(getResampledImage(curRes, 20, extractor, false));
            } else {
                if((*it).Name.compare("B2") == 0 || (*it).Name.compare("B3") == 0 || (*it).Name.compare("B4") == 0) {
                    // resample to 10m
                    m_ImageListRes10->PushBack(getResampledImage(curRes, 10, extractor, false));
                }
                else
                {
                    if((*it).Name.compare("B5") == 0 || (*it).Name.compare("B7") == 0 || (*it).Name.compare("B8") == 0) {
                        // resample to 20m
                        m_ImageListRes20->PushBack(getResampledImage(curRes, 20, extractor, false));
                    }
                }
            }
        }

        imageFile = cloudXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerCloud = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerCloud->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        m_ImageCloudRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageCloudRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageCloudResOrig = extractor->GetOutput();

        imageFile = waterXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerWater = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerWater->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        m_ImageWaterRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageWaterRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageWaterResOrig = extractor->GetOutput();

        imageFile = snowXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerSnow = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerSnow->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        m_ImageSnowRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageSnowRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageSnowResOrig = extractor->GetOutput();

        imageFile = aotXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerAot = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerAot->GetOutput() );
        extractor->SetChannel( nAotBandIdx );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        m_ImageAotRes10 = getResampledImage(curRes, 10, extractor, true);
        m_ImageAotRes20 = getResampledImage(curRes, 20, extractor, true);
        m_ImageAotResOrig = extractor->GetOutput();

        return true;
    }

    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask) {
         ResampleFilterType::Pointer resampler = ResampleFilterType::New();
         resampler->SetInput(image);

         // Set the interpolator
         if(isMask) {
             NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
             resampler->SetInterpolator(interpolator);
         }
         else {
            LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
            resampler->SetInterpolator(interpolator);

            //BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
            //interpolator->SetRadius(2);
            //resampler->SetInterpolator(interpolator);
         }

         IdentityTransformType::Pointer transform = IdentityTransformType::New();

         resampler->SetOutputParametersFromImage( image );
         // Scale Transform
         OutputVectorType scale;
         scale[0] = 1.0 / ratio;
         scale[1] = 1.0 / ratio;

         // Evaluate spacing
         InternalImageType::SpacingType spacing = image->GetSpacing();
         InternalImageType::SpacingType OutputSpacing;
         OutputSpacing[0] = spacing[0] * scale[0];
         OutputSpacing[1] = spacing[1] * scale[1];

         resampler->SetOutputSpacing(OutputSpacing);

         FloatVectorImageType::PointType origin = image->GetOrigin();
         FloatVectorImageType::PointType outputOrigin;
         outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
         outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

         resampler->SetOutputOrigin(outputOrigin);

         resampler->SetTransform(transform);

         // Evaluate size
         ResampleFilterType::SizeType recomputedSize;
         recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
         recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

         resampler->SetOutputSize(recomputedSize);

         m_ResamplersList->PushBack(resampler);
         return resampler;
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

    std::string getSpot4AotFileName(const std::unique_ptr<SPOT4Metadata>& meta)
    {
        // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
        // if the key is not present in the XML
        std::string fileName;
        if(meta->Files.OrthoSurfAOT == "") {
            std::string orthoSurf = meta->Files.OrthoSurfCorrPente;
            if(orthoSurf.empty()) {
                orthoSurf = meta->Files.OrthoSurfCorrEnv;
                if(!orthoSurf.empty()) {
                    int nPos = orthoSurf.find("ORTHO_SURF_CORR_ENV");
                    orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_ENV"), "AOT");
                    fileName = orthoSurf;
                }
            } else {
                int nPos = orthoSurf.find("ORTHO_SURF_CORR_PENTE");
                orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_PENTE"), "AOT");
                fileName = orthoSurf;
            }
        } else {
            fileName = meta->Files.OrthoSurfAOT;
        }

        return m_DirName + "/" + fileName;
    }

    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                 ExtractROIFilterType::Pointer extractor,
                                                 bool bIsMask) {
        if(nCurRes == nDesiredRes)
            return extractor->GetOutput();
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), fMultiplicationFactor, bIsMask);
        return resampler->GetOutput();
    }



    ListConcatenerFilterType::Pointer     m_ConcatenerRes10;
    ListConcatenerFilterType::Pointer     m_ConcatenerRes20;
    ListConcatenerFilterType::Pointer     m_ConcatenerResOrig;

    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    InternalImageListType::Pointer        m_ImageListRes10;
    InternalImageListType::Pointer        m_ImageListRes20;
    InternalImageListType::Pointer        m_ImageListResOrig;

    InternalImageType::Pointer            m_ImageCloudRes10;
    InternalImageType::Pointer            m_ImageWaterRes10;
    InternalImageType::Pointer            m_ImageSnowRes10;
    InternalImageType::Pointer            m_ImageAotRes10;

    InternalImageType::Pointer            m_ImageCloudRes20;
    InternalImageType::Pointer            m_ImageWaterRes20;
    InternalImageType::Pointer            m_ImageSnowRes20;
    InternalImageType::Pointer            m_ImageAotRes20;

    InternalImageType::Pointer            m_ImageCloudResOrig;
    InternalImageType::Pointer            m_ImageWaterResOrig;
    InternalImageType::Pointer            m_ImageSnowResOrig;
    InternalImageType::Pointer            m_ImageAotResOrig;

    ExtractROIFilterType::Pointer         m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer     m_ChannelExtractorList;
    ResampleFilterListType::Pointer       m_ResamplersList;
    std::string                           m_DirName;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ResampleAtS2Res)



