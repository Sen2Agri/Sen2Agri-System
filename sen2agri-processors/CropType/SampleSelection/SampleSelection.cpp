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

//    INPUTS: {reference polygons}, {sample ratio}
//    OUTPUTS: {training polygons}, {validation_polygons}

// The sample selection consists in splitting the reference data into 2 disjoint sets, the training
// set and the validation set.
// These sets are composed of polygons, not individual pixels.

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbOGRDataSourceToLabelImageFilter.h"

namespace otb
{

namespace Wrapper
{
class SampleSelection : public Application
{
public:
    typedef SampleSelection Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(SampleSelection, otb::Application);

private:
    void DoInit()
    {
        SetName("SampleSelection");
        SetDescription("Split the reference data into 2 disjoint sets, the training set and the "
                       "validation set.");

        SetDocName("SampleSelection");
        SetDocLongDescription(
            "The sample selection consists in splitting the reference data into 2 disjoint sets, "
            "the training set and the validation set. "
            "These sets are composed of polygons, not individual pixels.");
        SetDocLimitations("None");
        SetDocAuthors("LBU");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        // The input parameters:
        // - ref: Vector file containing reference data
        // - ratio: Ratio between the number of training and validation polygons per class (dafault:
        // 0.75)
        // The output parameters:
        // - tp: Vector file containing reference data for training
        // - vp: Vector file containing reference data for validation

        AddParameter(ParameterType_InputFilename, "ref", "Reference Polygons");
        AddParameter(ParameterType_Float, "ratio", "Sample Ratio");
        AddParameter(ParameterType_Int, "seed", "Seed for the random number generation");
        AddParameter(
            ParameterType_Empty, "nofilter", "Do not filter the polygons based on Crop/No crop");
        MandatoryOff("nofilter");

        AddParameter(ParameterType_OutputFilename, "tp", "Training Polygons");
        AddParameter(ParameterType_OutputFilename, "vp", "Validation Polygons");

        SetDefaultParameterFloat("ratio", 0.75);
        SetDefaultParameterInt("seed", std::time(0));

        SetDocExampleParameterValue("ref", "reference_polygons.shp");
        SetDocExampleParameterValue("ratio", "0.75");
        SetDocExampleParameterValue("seed", "0");
        SetDocExampleParameterValue("tp", "training_polygons.shp");
        SetDocExampleParameterValue("vp", "validation_polygons.shp");
    }

    void DoUpdateParameters()
    {
    }

    // The algorithm consists in a random sampling without replacement of the polygons of each class
    // with
    // probability p = sample_ratio value for the training set and
    // 1 - p for the validation set.
    void DoExecute()
    {
        // Internal variables for accessing the files
        otb::ogr::DataSource::Pointer ogrRef;
        otb::ogr::DataSource::Pointer ogrTp;
        otb::ogr::DataSource::Pointer ogrVp;

        std::multimap<int, ogr::Feature> featuresMap;

        // Create the reader over the reference file
        ogrRef =
            otb::ogr::DataSource::New(GetParameterString("ref"), otb::ogr::DataSource::Modes::Read);
        if (ogrRef->GetLayersCount() < 1) {
            itkExceptionMacro("The source file must contain at least one layer");
        }

        // get the required sampling ratio
        const float ratio = GetParameterFloat("ratio");
        // get the random seed
        const int seed = GetParameterInt("seed");

        // Create the writers over the outut files
        ogrTp = otb::ogr::DataSource::New(GetParameterString("tp"),
                                          otb::ogr::DataSource::Modes::Overwrite);
        ogrVp = otb::ogr::DataSource::New(GetParameterString("vp"),
                                          otb::ogr::DataSource::Modes::Overwrite);

        // read the layer of the reference file
        otb::ogr::Layer sourceLayer = ogrRef->GetLayer(0);
        if (sourceLayer.GetGeomType() != wkbPolygon) {
            itkExceptionMacro("The first layer must contain polygons!");
        }
        auto filter = !GetParameterEmpty("nofilter");
        if (filter) {
            std::cout << "Excluding non-crop features\n";
            auto ret = sourceLayer.ogr().SetAttributeFilter("CROP=1");
            if (ret != OGRERR_NONE) {
                std::cerr << "SetAttributeFilter() failed: " << ret << '\n';
            }
        }

        // read all features from the source field and add them to the multimap
        for (ogr::Feature &feature : sourceLayer) {
            if (!filter || feature.ogr().GetFieldAsInteger("CROP")) {
                featuresMap.insert(std::pair<int, ogr::Feature>(
                    feature.ogr().GetFieldAsInteger("CODE"), feature.Clone()));
            }
        }
        std::cerr << '\n';

        // create the layers for the target files
        otb::ogr::Layer tpLayer = ogrTp->CreateLayer(
            sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());
        otb::ogr::Layer vpLayer = ogrVp->CreateLayer(
            sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());

        // add the fields
        OGRFeatureDefn &layerDefn = sourceLayer.GetLayerDefn();

        for (int i = 0; i < layerDefn.GetFieldCount(); i++) {
            OGRFieldDefn &fieldDefn = *layerDefn.GetFieldDefn(i);
            tpLayer.CreateField(fieldDefn);
            vpLayer.CreateField(fieldDefn);
        }

        int lastcode = -1;
        int featTrainingTarget = 0;
        int featValidationTarget = 0;
        int featTrainingCount = 0;
        int featValidationCount = 0;

        // initialise the random number generator
        std::srand(seed);
        // Loop through the entries
        std::multimap<int, ogr::Feature>::iterator it;
        for (it = featuresMap.begin(); it != featuresMap.end(); ++it) {
            // get the feature
            ogr::Feature &f = it->second;

            if (it->first != lastcode) {
                lastcode = it->first;

                // get the number of features with the same code
                int featCount = featuresMap.count(lastcode);

                // compute the target training features
                featTrainingTarget = std::max(1, (int)std::round((float)featCount * ratio));
                featValidationTarget = featCount - featTrainingTarget;
                featTrainingCount = 0;
                featValidationCount = 0;

                // Add info message to log
                otbAppLogINFO("Found " << featCount << " features with CODE = " << lastcode << ". "
                                       << "Using " << featTrainingTarget << " for training and "
                                       << featValidationTarget << " for validation. ");
            }

            // generate a random number and convert it to the [0..1] interval
            float random = (float)std::rand() / (float)RAND_MAX;

            // select the target file for this feature
            if (f.GetGeometry()) {
                if ((random <= ratio && featTrainingCount < featTrainingTarget) ||
                    featValidationCount == featValidationTarget) {
                    tpLayer.CreateFeature(f.Clone());
                    featTrainingCount++;
                } else {
                    vpLayer.CreateFeature(f.Clone());
                    featValidationCount++;
                }
            } else {
                otbAppLogWARNING("Feature " << f.ogr().GetFieldAsInteger("ID")
                                            << " has no associated geometry");
            }
        }

        ogrTp->SyncToDisk();
        ogrVp->SyncToDisk();

        featTrainingCount = tpLayer.GetFeatureCount(true);
        featValidationCount = vpLayer.GetFeatureCount(true);
        otbAppLogINFO("Total features: " << featTrainingCount + featValidationCount
                                         << ", training features: " << featTrainingCount
                                         << ", validation features: " << featValidationCount);
    }
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::SampleSelection)
