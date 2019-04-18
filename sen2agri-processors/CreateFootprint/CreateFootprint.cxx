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

#include "MetadataHelperFactory.h"

//#include "MetadataUtil.hpp"

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

        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<int>(file);
        std::vector<int> relBandIdxs;
        const std::vector<std::string> &bandNames = m_pHelper->GetBandNamesForResolution(m_pHelper->GetProductResolutions()[0]);
        // If the product readers have several bands in one raster (like MACCS) it will return the full raster image
        // and the relative band idx will be updated accordingly.
        // If we have bands in distinct files, then we are interested in only one file (no need to concatenate all bands for the first resolution)
        MetadataHelper<int>::VectorImageType::Pointer img = m_pHelper->GetImage({bandNames[0]}, &relBandIdxs);
        img->UpdateOutputInformation();

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

            PolygonType::Pointer polygon = FootprintFromGeoCoding<int>(img);

            vectorData->SetProjectionRef(img->GetProjectionRef());

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
            if (HasValue("outbounds")) {
                auto polygon = FootprintFromGeoCoding<int>(img);

                auto sourceSrs = static_cast<OGRSpatialReference *>(
                    OSRNewSpatialReference(img->GetProjectionRef().c_str()));
                if (!sourceSrs) {
                    itkExceptionMacro(
                        << "Unable to create source OGRSpatialReference from OGR WKT\n"
                        << img->GetProjectionRef());
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
                                          << img->GetProjectionRef()
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
            m_MaskFilter->SetInput(img);

            m_PolygonizeFilter = PolygonizeFilterType::New();
            m_PolygonizeFilter->SetInput(m_MaskFilter->GetOutput());
            m_PolygonizeFilter->SetInputMask(m_MaskFilter->GetOutput());

            SetParameterOutputVectorData("out", m_PolygonizeFilter->GetOutput());
        }
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

    ContinuousIndexType PointFromVector(const FloatVectorImageType::VectorType &v)
    {
        ContinuousIndexType index;
        index[0] = v[0];
        index[1] = v[1];
        return index;
    }

    std::unique_ptr<MetadataHelper<int>> m_pHelper;
    MaskFilterType::Pointer m_MaskFilter;
    PolygonizeFilterType::Pointer m_PolygonizeFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CreateFootprint)
