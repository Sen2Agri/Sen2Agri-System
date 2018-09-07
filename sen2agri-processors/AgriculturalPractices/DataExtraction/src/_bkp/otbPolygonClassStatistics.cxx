/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbOGRDataToClassStatisticsFilter.h"
#include "otbStatisticsXMLFileWriter.h"
#include "otbGeometriesProjectionFilter.h"
#include "otbGeometriesSet.h"
#include "otbWrapperElevationParametersHandler.h"
#include "otbConcatenateVectorImagesFilter.h"
#include "otbBandMathImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include <boost/filesystem.hpp>

namespace otb
{
namespace Wrapper
{

/** Utility function to negate std::isalnum */
bool IsNotAlphaNum(char c)
{
    return !std::isalnum(c);
}

class PolygonClassStatistics : public Application
{
public:
    /** Standard class typedefs. */
    typedef PolygonClassStatistics        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(PolygonClassStatistics, otb::Application);

    /** Filters typedef */
    typedef float                                           PixelType;
    typedef otb::Image<PixelType, 2>                        SimpleImageType;

    typedef FloatVectorImageType                            ImageType;
    typedef UInt8ImageType                                  MaskImageType;

    typedef otb::OGRDataToClassStatisticsFilter<ImageType,UInt8ImageType> FilterType;

    typedef otb::StatisticsXMLFileWriter<ImageType::PixelType> StatWriterType;

    typedef otb::GeometriesSet GeometriesType;

    typedef otb::GeometriesProjectionFilter ProjectionFilterType;

    typedef otb::ConcatenateVectorImagesFilter<ImageType>               ConcatenateImagesFilterType;
    typedef otb::ImageFileReader<ImageType>                             ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                            ReadersListType;

    typedef std::map<std::string , std::string>           GenericMapType;
    typedef std::map<std::string , GenericMapType>        GenericMapContainer;

private:
    PolygonClassStatistics()
    {
        m_concatenateImagesFilter = ConcatenateImagesFilterType::New();
        m_Readers = ReadersListType::New();
    }

    void DoInit() override
    {
        SetName("PolygonClassStatistics");
        SetDescription("Computes statistics on a training polygon set.");

        // Documentation
        SetDocName("Polygon Class Statistics");
        SetDocLongDescription("The application processes a set of geometries "
        "intended for training (they should have a field giving the associated "
        "class). The geometries are analyzed against a support image to compute "
        "statistics : \n"
        "  - number of samples per class\n"
        "  - number of samples per geometry\n"
        "An optional raster mask can be used to discard samples. Different types"
        " of geometry are supported : polygons, lines, points. The behaviour is "
        "different for each type of geometry :\n"
        "  - polygon: select pixels whose center is inside the polygon\n"
        "  - lines  : select pixels intersecting the line\n"
        "  - points : select closest pixel to the point");
        SetDocLimitations("None");
        SetDocAuthors("OTB-Team");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);

//        AddParameter(ParameterType_InputImage,  "in",   "Input image");
//        SetParameterDescription("in", "Support image that will be classified");

        AddParameter(ParameterType_InputImageList, "il", "Input Images");
        SetParameterDescription("il", "Support images that will be classified");

        AddParameter(ParameterType_InputImage,  "mask",   "Input validity mask");
        SetParameterDescription("mask", "Validity mask (only pixels corresponding to a mask value greater than 0 will be used for statistics)");
        MandatoryOff("mask");

        AddParameter(ParameterType_InputFilename, "vec", "Input vectors");
        SetParameterDescription("vec","Input geometries to analyze");

        AddParameter(ParameterType_OutputFilename, "out", "Output XML statistics file");
        SetParameterDescription("out","Output file to store statistics (XML format)");

        AddParameter(ParameterType_ListView, "field", "Field Name");
        SetParameterDescription("field","Name of the field carrying the class name in the input vectors.");
        //SetListViewSingleSelectionMode("field",true);

        AddParameter(ParameterType_Int, "layer", "Layer Index");
        SetParameterDescription("layer", "Layer index to read in the input vector file.");
        MandatoryOff("layer");
        SetDefaultParameterInt("layer",0);

        AddParameter(ParameterType_String, "exp", "Expression to apply for the pixels preprocessing");
        SetParameterDescription("exp", "Expression to apply for the pixels preprocessing.");
        MandatoryOff("exp");

        ElevationParametersHandler::AddElevationParameters(this, "elev");

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue("in", "support_image.tif");
        SetDocExampleParameterValue("vec", "variousVectors.sqlite");
        SetDocExampleParameterValue("field", "label");
        SetDocExampleParameterValue("out","polygonStat.xml");

        //SetOfficialDocLink();
    }

    void DoUpdateParameters() override
    {
        if ( HasValue("vec") )
        {
            std::string vectorFile = GetParameterString("vec");
            ogr::DataSource::Pointer ogrDS =
            ogr::DataSource::New(vectorFile, ogr::DataSource::Modes::Read);
            ogr::Layer layer = ogrDS->GetLayer(this->GetParameterInt("layer"));
            ogr::Feature feature = layer.ogr().GetNextFeature();

            ClearChoices("field");

            for(int iField=0; iField<feature.ogr().GetFieldCount(); iField++)
            {
                std::string key, item = feature.ogr().GetFieldDefnRef(iField)->GetNameRef();
                key = item;
                std::string::iterator end = std::remove_if(key.begin(),key.end(),IsNotAlphaNum);
                std::transform(key.begin(), end, key.begin(), tolower);

                OGRFieldType fieldType = feature.ogr().GetFieldDefnRef(iField)->GetType();

                if(fieldType == OFTString || fieldType == OFTInteger/* || ogr::version_proxy::IsOFTInteger64(fieldType)*/)
                {
                    std::string tmpKey="field."+key.substr(0, end - key.begin());
                    AddChoice(tmpKey,item);
                }
            }
        }

        // Check that the extension of the output parameter is XML (mandatory for
        // StatisticsXMLFileWriter)
        // Check it here to trigger the error before polygons analysis

        if ( HasValue("out") )
        {
            // Store filename extension
            // Check that the right extension is given : expected .xml
            const std::string extension = itksys::SystemTools::GetFilenameLastExtension(this->GetParameterString("out"));

            if (itksys::SystemTools::LowerCase(extension) != ".xml")
            {
                otbAppLogFATAL( << extension << " is a wrong extension for parameter \"out\": Expected .xml" );
            }
        }
    }

    void DoExecute() override
    {
        otb::ogr::DataSource::Pointer vectors = otb::ogr::DataSource::New(this->GetParameterString("vec"));
        // Retrieve the field name
        const std::vector<int> &selectedCFieldIdx = GetSelectedItems("field");
        if(selectedCFieldIdx.empty())
        {
            otbAppLogFATAL(<<"No field has been selected for data labelling!");
        }

        const std::vector<std::string> &imagesPaths = this->GetParameterStringList("il");
        if(imagesPaths.size() == 0) {
            otbAppLogFATAL(<<"No image was given as input!");
        }

        otb::Wrapper::ElevationParametersHandler::SetupDEMHandlerFromElevationParameters(this,"elev");

        // Reproject geometries
        //FloatVectorImageType::Pointer inputImg = this->GetParameterImage("in");
        FloatVectorImageType::Pointer inputImg = getInputImage(imagesPaths[0]);
        const std::string &imageProjectionRef = inputImg->GetProjectionRef();
        FloatVectorImageType::ImageKeywordlistType imageKwl = inputImg->GetImageKeywordlist();
        const std::string &vectorProjectionRef = vectors->GetLayer(GetParameterInt("layer")).GetProjectionRef();

        otb::ogr::DataSource::Pointer reprojVector = vectors;
        const OGRSpatialReference imgOGRSref = OGRSpatialReference( imageProjectionRef.c_str() );
        const OGRSpatialReference vectorOGRSref = OGRSpatialReference( vectorProjectionRef.c_str() );
        bool doReproj = true;
        // don't reproject for these cases
        if (  vectorProjectionRef.empty() || imgOGRSref.IsSame( &vectorOGRSref )
            || ( imageProjectionRef.empty() && imageKwl.GetSize() == 0) ) {
            doReproj = false;
        }

        GeometriesType::Pointer inputGeomSet;
        GeometriesType::Pointer outputGeomSet;
        ProjectionFilterType::Pointer geometriesProjFilter;
        if (doReproj)
        {
            inputGeomSet = GeometriesType::New(vectors);
            reprojVector = otb::ogr::DataSource::New();
            outputGeomSet = GeometriesType::New(reprojVector);
            // Filter instantiation
            geometriesProjFilter = ProjectionFilterType::New();
            geometriesProjFilter->SetInput(inputGeomSet);
            if (imageProjectionRef.empty())
            {
                geometriesProjFilter->SetOutputKeywordList(inputImg->GetImageKeywordlist()); // nec qd capteur
            }
            geometriesProjFilter->SetOutputProjectionRef(imageProjectionRef);
            geometriesProjFilter->SetOutput(outputGeomSet);
            otbAppLogINFO("Reprojecting input vectors...");
            geometriesProjFilter->Update();
        }

        const std::vector<std::string> &cFieldNames = GetChoiceNames("field");
        const std::string &fieldName = cFieldNames[selectedCFieldIdx.front()];

        std::vector<std::string>::const_iterator itImages;
        std::vector<std::string>::const_iterator itImagesEnd = imagesPaths.end();

        StatWriterType::Pointer statWriter = StatWriterType::New();
        statWriter->SetFileName(this->GetParameterString("out"));
        int i = 1;
        for (itImages = imagesPaths.begin(); itImages != itImagesEnd; ++itImages)
        {
            FilterType::Pointer filter = getStatisticsFilter(*itImages, reprojVector, fieldName);

            FilterType::ClassCountMapType &classCount = filter->GetClassCountOutput()->Get();
            FilterType::PolygonSizeMapType &polySize = filter->GetPolygonSizeOutput()->Get();

            FilterType::PixelValueMapType meanValues = filter->GetMeanValueMap();
            FilterType::PixelValueMapType stdDevValues = filter->GetStandardDeviationValueMap();
            FilterType::PixelValueMapType minValues = filter->GetMinValueMap();
            FilterType::PixelValueMapType maxValues = filter->GetMaxValueMap();

//            boost::filesystem::path p(outParamVal);
//            const boost::filesystem::path &simpleFilePath = p.stem();
//            const std::string &newFileName = p.parent_path().generic_string() + simpleFilePath.generic_string() +
//                    "_" + std::to_string(i) + p.extension().generic_string();
            statWriter->AddInputMap<FilterType::ClassCountMapType>(*itImages, "samplesPerClass",classCount);
            statWriter->AddInputMap<FilterType::PolygonSizeMapType>(*itImages, "samplesPerVector",polySize);

            statWriter->AddInputMap<FilterType::PixelValueMapType>(*itImages, "meanValues",meanValues);
            statWriter->AddInputMap<FilterType::PixelValueMapType>(*itImages, "stdDevValues",stdDevValues);
            statWriter->AddInputMap<FilterType::PixelValueMapType>(*itImages, "minValues",minValues);
            statWriter->AddInputMap<FilterType::PixelValueMapType>(*itImages, "maxValues",maxValues);

            i++;
        }
        statWriter->Update();
//        if (imagesPaths.size() > 1)
//        {
//            std::vector<std::string>::const_iterator itImages;
//            std::vector<std::string>::const_iterator itImagesEnd = imagesPaths.end();
//            for (itImages = imagesPaths.begin(); itImages != itImagesEnd; ++itImages)
//            {
//                FloatVectorImageType::Pointer input = getInputImage(*itImages);
//                m_concatenateImagesFilter->PushBackInput(input);
//            }
//            filter->SetInput(m_concatenateImagesFilter->GetOutput());
//        } else {
//            filter->SetInput(getInputImage(imagesPaths[0]));
//        }

    }

    FilterType::Pointer getStatisticsFilter(const std::string &filePath,
                                            otb::ogr::DataSource::Pointer reprojVector,
                                            const std::string &fieldName)
    {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(getInputImage(filePath));

        if (IsParameterEnabled("mask") && HasValue("mask"))
        {
            filter->SetMask(this->GetParameterImage<UInt8ImageType>("mask"));
        }
        if (IsParameterEnabled("exp") && HasValue("exp"))
        {
            filter->SetExpression(GetParameterString("exp"));
        }

        filter->SetOGRData(reprojVector);
        filter->SetFieldName(fieldName);
        filter->SetLayerIndex(this->GetParameterInt("layer"));
        filter->GetStreamer()->SetAutomaticAdaptativeStreaming(GetParameterInt("ram"));

        AddProcess(filter->GetStreamer(),"Analyze polygons...");
        filter->Update();

        return filter;
    }

    FloatVectorImageType::Pointer getInputImage(const std::string &imgPath) {
        ImageReaderType::Pointer imageReader = ImageReaderType::New();
        m_Readers->PushBack(imageReader);
        imageReader->SetFileName(imgPath);
        imageReader->UpdateOutputInformation();
        FloatVectorImageType::Pointer retImg = imageReader->GetOutput();
        retImg->UpdateOutputInformation();
        return retImg;
    }

    private:
        ConcatenateImagesFilterType::Pointer m_concatenateImagesFilter;
        ReadersListType::Pointer m_Readers;
        GenericMapContainer         m_GenericMapContainer;
};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::PolygonClassStatistics)
