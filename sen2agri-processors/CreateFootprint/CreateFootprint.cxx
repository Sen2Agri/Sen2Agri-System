#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbBandMathImageFilter.h"
#include "otbVectorDataFileWriter.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbStreamingStatisticsImageFilter.h"
#include "otbLabelImageToVectorDataFilter.h"

// Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "otbOGRIOHelper.h"
#include "otbGeoInformationConversion.h"

#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
#include "MetadataUtil.hpp"

namespace otb
{

namespace Wrapper
{

template <typename TInput, typename TOutput>
class CreateMaskFunctor
{
public:
    typedef typename TInput::ValueType InputValueType;

    CreateMaskFunctor() : m_NoDataValue()
    {
    }

    TOutput operator()(const TInput &in)
    {
        bool ok = false;

        int size = in.GetSize();
        for (int i = 0; i < size; i++) {
            if (in[i] != -10000) {
                ok = true;
                break;
            }
        }

        return ok ? 1 : 0;
    }

    void SetNoDataValue(InputValueType value)
    {
        m_NoDataValue = value;
    }

private:
    InputValueType m_NoDataValue;
};

class CreateFootprint : public Application
{
public:
    typedef CreateFootprint Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(CreateFootprint, otb::Application)

private:
    typedef VectorDataType::DataNodeType DataNodeType;
    typedef VectorDataType::DataTreeType DataTreeType;
    typedef DataNodeType::PolygonType PolygonType;
    typedef PolygonType::ContinuousIndexType ContinuousIndexType;

    typedef Int32VectorImageType InputImageType;
    typedef Int32ImageType MaskImageType;
    typedef ImageFileReader<InputImageType> ImageFileReaderType;
    typedef itk::UnaryFunctorImageFilter<
        InputImageType,
        MaskImageType,
        CreateMaskFunctor<InputImageType::PixelType, MaskImageType::PixelType> > MaskFilterType;
    typedef otb::LabelImageToVectorDataFilter<MaskImageType> PolygonizeFilterType;

    void DoInit() ITK_OVERRIDE
    {
        SetName("CreateFootprint");
        SetDescription("Creates vector data from an image footprint");

        SetDocName("CreateFootprint");
        SetDocLongDescription(
            "Creates vector data with a polygon determined by the footprint of an image");
        SetDocLimitations("None");
        SetDocAuthors("LNI");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputFilename,
                     "in",
                     "The input file, an image, or a SPOT4 or MACCS descriptor file");
        AddParameter(ParameterType_Choice, "mode", "Mode");
        SetParameterDescription("mode", "Specifies what the footprint will contain");
        AddChoice("mode.metadata", "Footprint from metadata");
        SetParameterDescription("mode.metadata", "Uses the bounding box from the metadata");
        AddChoice("mode.raster", "Footprint from raster");
        SetParameterDescription("mode.metadata",
                                "Uses the raster 'no data' values to create covering polygons");

        AddParameter(ParameterType_Int, "mode.raster.nodata", "'No data' value");
        SetDefaultParameterInt("mode.raster.nodata", -10000);

        AddParameter(ParameterType_OutputVectorData, "out", "The output footprint");

        SetDocExampleParameterValue("in", "image.tif");
        SetDocExampleParameterValue("mode", "metadata");
        SetDocExampleParameterValue("out", "footprint.shp");
    }

    void DoUpdateParameters() ITK_OVERRIDE
    {
    }

    void DoExecute() ITK_OVERRIDE
    {
        const auto &file = GetParameterString("in");

        if (GetParameterString("mode") == "metadata") {
            auto vectorData = CreateVectorDataFromMetadata(file);

            SetParameterOutputVectorData("out", vectorData.GetPointer());
        } else {
            m_ImageFileReader = ImageFileReaderType::New();
            m_ImageFileReader->SetFileName(getMainRasterFile(file));

            m_MaskFilter = MaskFilterType::New();
            m_MaskFilter->GetFunctor().SetNoDataValue(GetParameterInt("mode.raster.nodata"));
            m_MaskFilter->SetInput(m_ImageFileReader->GetOutput());

            m_PolygonizeFilter = PolygonizeFilterType::New();
            m_PolygonizeFilter->SetInput(m_MaskFilter->GetOutput());
            m_PolygonizeFilter->SetInputMask(m_MaskFilter->GetOutput());

            SetParameterOutputVectorData("out", m_PolygonizeFilter->GetOutput());
        }
    }

    VectorDataType::Pointer CreateVectorDataFromMetadata(const std::string &file)
    {
        auto vectorData = VectorDataType::New();

        auto document = DataNodeType::New();
        document->SetNodeType(otb::DOCUMENT);
        document->SetNodeId("footprint");
        auto folder = DataNodeType::New();
        folder->SetNodeType(otb::FOLDER);
        auto polygonNode = DataNodeType::New();
        polygonNode->SetNodeType(otb::FEATURE_POLYGON);

        auto tree = vectorData->GetDataTree();
        auto root = tree->GetRoot()->Get();

        tree->Add(document, root);
        tree->Add(folder, document);
        tree->Add(polygonNode, folder);

        PolygonType::Pointer polygon;
        if (auto metadata = itk::MACCSMetadataReader::New()->ReadMetadata(file)) {
            polygon = FootprintFromMACCSMetadata(*metadata);

            vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
        } else if (auto metadata = itk::SPOT4MetadataReader::New()->ReadMetadata(file)) {
            polygon = FootprintFromSPOT4Metadata(*metadata);

            vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
        } else {
            auto reader = otb::ImageFileReader<FloatVectorImageType>::New();
            reader->SetFileName(file);
            reader->UpdateOutputInformation();

            auto output = reader->GetOutput();

            polygon = FootprintFromGeoCoding(output);

            vectorData->SetProjectionRef(output->GetProjectionRef());
        }

        polygonNode->SetPolygonExteriorRing(polygon);

        return vectorData;
    }

    VectorDataType::Pointer CreateVectorDataFromRaster(const std::string &file)
    {
        auto vectorData = VectorDataType::New();

        auto document = DataNodeType::New();
        document->SetNodeType(otb::DOCUMENT);
        document->SetNodeId("footprint");
        auto folder = DataNodeType::New();
        folder->SetNodeType(otb::FOLDER);
        auto polygonNode = DataNodeType::New();
        polygonNode->SetNodeType(otb::FEATURE_POLYGON);

        auto tree = vectorData->GetDataTree();
        auto root = tree->GetRoot()->Get();

        tree->Add(document, root);
        tree->Add(folder, document);
        tree->Add(polygonNode, folder);

        PolygonType::Pointer polygon;
        if (auto metadata = itk::MACCSMetadataReader::New()->ReadMetadata(file)) {
            polygon = FootprintFromMACCSMetadata(*metadata);

            vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
        } else if (auto metadata = itk::SPOT4MetadataReader::New()->ReadMetadata(file)) {
            polygon = FootprintFromSPOT4Metadata(*metadata);

            vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
        } else {
            auto reader = otb::ImageFileReader<FloatVectorImageType>::New();
            reader->SetFileName(file);
            reader->UpdateOutputInformation();

            auto output = reader->GetOutput();

            polygon = FootprintFromGeoCoding(output);

            vectorData->SetProjectionRef(output->GetProjectionRef());
        }

        polygonNode->SetPolygonExteriorRing(polygon);

        return vectorData;
    }

    PolygonType::Pointer FootprintFromMACCSMetadata(const MACCSFileMetadata &metadata)
    {
        auto poly = PolygonType::New();
        poly->AddVertex(
            PointFromMACCSGeopoint(metadata.ProductInformation.GeoCoverage.UpperLeftCorner));
        poly->AddVertex(
            PointFromMACCSGeopoint(metadata.ProductInformation.GeoCoverage.LowerLeftCorner));
        poly->AddVertex(
            PointFromMACCSGeopoint(metadata.ProductInformation.GeoCoverage.LowerRightCorner));
        poly->AddVertex(
            PointFromMACCSGeopoint(metadata.ProductInformation.GeoCoverage.UpperRightCorner));
        return poly;
    }

    PolygonType::Pointer FootprintFromSPOT4Metadata(const SPOT4Metadata &metadata)
    {
        auto poly = PolygonType::New();
        poly->AddVertex(PointFromSPOT4Point(metadata.WGS84.HGX, metadata.WGS84.HGY));
        poly->AddVertex(PointFromSPOT4Point(metadata.WGS84.BGX, metadata.WGS84.BGY));
        poly->AddVertex(PointFromSPOT4Point(metadata.WGS84.BDX, metadata.WGS84.BDY));
        poly->AddVertex(PointFromSPOT4Point(metadata.WGS84.HDX, metadata.WGS84.HDY));
        return poly;
    }

    PolygonType::Pointer FootprintFromGeoCoding(const FloatVectorImageType *image)
    {
        auto poly = PolygonType::New();
        poly->AddVertex(PointFromVector(image->GetUpperLeftCorner()));
        poly->AddVertex(PointFromVector(image->GetLowerLeftCorner()));
        poly->AddVertex(PointFromVector(image->GetLowerRightCorner()));
        poly->AddVertex(PointFromVector(image->GetUpperRightCorner()));
        return poly;
    }

    ContinuousIndexType PointFromMACCSGeopoint(const MACCSGeoPoint &point)
    {
        ContinuousIndexType index;
        index[0] = point.Long;
        index[1] = point.Lat;
        return index;
    }

    ContinuousIndexType PointFromSPOT4Point(double x, double y)
    {
        ContinuousIndexType index;
        index[0] = x;
        index[1] = y;
        return index;
    }

    ContinuousIndexType PointFromVector(const FloatVectorImageType::VectorType &v)
    {
        ContinuousIndexType index;
        index[0] = v[0];
        index[1] = v[1];
        return index;
    }

    ImageFileReaderType::Pointer m_ImageFileReader;
    MaskFilterType::Pointer m_MaskFilter;
    PolygonizeFilterType::Pointer m_PolygonizeFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CreateFootprint)
