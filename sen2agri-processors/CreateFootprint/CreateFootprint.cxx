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
 
#include <limits>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbVectorImage.h"

#include "otbLabelImageToVectorDataFilter.h"

#include "otbOGRIOHelper.h"
#include "otbGeoInformationConversion.h"

#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
#include "MetadataUtil.hpp"

#include "CreateMaskFromValueFunctor.hxx"

namespace otb
{

namespace Wrapper
{

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
        CreateMaskFromValueFunctor<InputImageType::PixelType, MaskImageType::PixelType> >
    MaskFilterType;
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

        AddParameter(
            ParameterType_OutputFilename, "outbounds", "The output footprint as a text file");
        MandatoryOff("outbounds");

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
            std::string rasterFileName = file;
            if (auto metadata = itk::MACCSMetadataReader::New()->ReadMetadata(file)) {
                //polygon = FootprintFromMACCSMetadata(*metadata);
                //vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
                std::string rootFolder = extractFolder(file);
                rasterFileName = getMACCSRasterFileName(rootFolder, metadata->ProductOrganization.ImageFiles, "_FRE");
                if (rasterFileName.size() == 0) {
                    rasterFileName = getMACCSRasterFileName(rootFolder, metadata->ProductOrganization.ImageFiles, "_FRE_R1");
                }
            } else if (auto metadata = itk::SPOT4MetadataReader::New()->ReadMetadata(file)) {
                //polygon = FootprintFromSPOT4Metadata(*metadata);
                //vectorData->SetProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
                std::string rootFolder = extractFolder(file);
                rasterFileName = rootFolder + metadata->Files.OrthoSurfCorrPente;
            }
            auto reader = otb::ImageFileReader<FloatVectorImageType>::New();
            reader->SetFileName(rasterFileName);
            reader->UpdateOutputInformation();

            auto output = reader->GetOutput();

            polygon = FootprintFromGeoCoding(output);

            vectorData->SetProjectionRef(output->GetProjectionRef());

            polygonNode->SetPolygonExteriorRing(polygon);

            SetParameterOutputVectorData("out", vectorData.GetPointer());

            if (HasValue("outbounds")) {
                std::ofstream boundsFile(GetParameterString("outbounds"));
                boundsFile.precision(std::numeric_limits<double>::max_digits10);

                for (const auto &v : *polygon->GetVertexList()) {
                    boundsFile << std::fixed << v[0] << ' ' << v[1] << '\n';
                }
            }
        } else {
            m_ImageFileReader = ImageFileReaderType::New();
            m_ImageFileReader->SetFileName(getMainRasterFile(file));

            if (HasValue("outbounds")) {
                m_ImageFileReader->UpdateOutputInformation();
                auto output = m_ImageFileReader->GetOutput();
                auto polygon = FootprintFromGeoCoding(output);

                auto sourceSrs = static_cast<OGRSpatialReference *>(
                    OSRNewSpatialReference(output->GetProjectionRef().c_str()));
                if (!sourceSrs) {
                    itkExceptionMacro(
                        << "Unable to create source OGRSpatialReference from OGR WKT\n"
                        << output->GetProjectionRef());
                }

                auto targetSrs =
                    static_cast<OGRSpatialReference *>(OSRNewSpatialReference(SRS_WKT_WGS84));
                if (!targetSrs) {
                    OSRDestroySpatialReference(sourceSrs);

                    itkExceptionMacro(<< "Unable to create WGS84 OGRSpatialReference");
                }

                auto transform = static_cast<OGRCoordinateTransformation *>(
                    OCTNewCoordinateTransformation(sourceSrs, targetSrs));
                if (!transform) {
                    OSRDestroySpatialReference(sourceSrs);
                    OSRDestroySpatialReference(targetSrs);

                    itkExceptionMacro(<< "Unable to create OGRCoordinateTransformation");
                }

                auto point = static_cast<OGRPoint *>(OGRGeometryFactory::createGeometry(wkbPoint));
                if (!point) {
                    OCTDestroyCoordinateTransformation(transform);
                    OSRDestroySpatialReference(targetSrs);
                    OSRDestroySpatialReference(sourceSrs);

                    itkExceptionMacro(<< "Unable to create OGRPoint");
                }

                std::ofstream boundsFile(GetParameterString("outbounds"));
                boundsFile.precision(std::numeric_limits<double>::max_digits10);

                for (const auto &v : *polygon->GetVertexList()) {
                    point->setX(v[0]);
                    point->setY(v[1]);
                    point->setZ(0.0);
                    auto err = point->transform(transform);
                    if (err == OGRERR_NONE) {
                        boundsFile << std::fixed << point->getX() << ' ' << point->getY() << '\n';
                    } else {
                        OGRGeometryFactory::destroyGeometry(point);

                        OCTDestroyCoordinateTransformation(transform);
                        OSRDestroySpatialReference(targetSrs);
                        OSRDestroySpatialReference(sourceSrs);

                        itkExceptionMacro(<< "Unable to transform point (" << v[0] << ", " << v[1]
                                          << ") to WGS84. The source projection is:\n"
                                          << output->GetProjectionRef()
                                          << "\nOGR returned error code " << err);
                    }
                }

                OGRGeometryFactory::destroyGeometry(point);

                OCTDestroyCoordinateTransformation(transform);
                OSRDestroySpatialReference(targetSrs);
                OSRDestroySpatialReference(sourceSrs);
            }

            m_MaskFilter = MaskFilterType::New();
            m_MaskFilter->GetFunctor().SetNoDataValue(GetParameterInt("mode.raster.nodata"));
            m_MaskFilter->SetInput(m_ImageFileReader->GetOutput());

            m_PolygonizeFilter = PolygonizeFilterType::New();
            m_PolygonizeFilter->SetInput(m_MaskFilter->GetOutput());
            m_PolygonizeFilter->SetInputMask(m_MaskFilter->GetOutput());

            SetParameterOutputVectorData("out", m_PolygonizeFilter->GetOutput());
        }
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

    template <typename TPixel>
    PolygonType::Pointer FootprintFromGeoCoding(const otb::VectorImage<TPixel> *image)
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

    // Extract the folder from a given path.
    std::string extractFolder(const std::string& filename) {
        size_t pos = filename.find_last_of("/\\");
        if (pos == std::string::npos) {
            return "";
        }

        return filename.substr(0, pos) + "/";
    }

    // Get the path to the Spot4 raster
    std::string getSPOT4RasterFileName(const SPOT4Metadata & desc, const std::string& folder) {
        return folder + desc.Files.OrthoSurfCorrPente;
    }

    // Return the path to a file for which the name end in the ending
    std::string getMACCSRasterFileName(const std::string& rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

        for (const MACCSFileInformation& fileInfo : imageFiles) {
            if (fileInfo.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
            }

        }
        return "";
    }

    ImageFileReaderType::Pointer m_ImageFileReader;
    MaskFilterType::Pointer m_MaskFilter;
    PolygonizeFilterType::Pointer m_PolygonizeFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CreateFootprint)
