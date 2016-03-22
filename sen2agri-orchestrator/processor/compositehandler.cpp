#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "compositehandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"

#define TasksNoPerProduct 7

void CompositeHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                                                  const ProductAvailableEvent &event)
{
    Q_UNUSED(ctx);
    // in this moment, we do not handle the products as they appear but maybe later it can be useful
    // for some other processors. The code below can be used as an example.
    return;
    // TODO: Here we must get all job IDs for the current processor id
    std::vector<int> jobIds;

    // for each job ID, check if the job accepts the current product
    for (unsigned int i = 0; i < jobIds.size(); i++) {
        // Check if the product can be processed by the current job
        if (IsProductAcceptableForJob(jobIds[i], event)) {
            // create a list with products that will have in this case only one element
            QStringList listProducts;
            // Get the product path from the productId
            QString strProductPath; // TODO = ctx.GetProductPathFromId(event.productId);
            listProducts.append(strProductPath);
            // process the received L2A product in the current job
            //HandleNewProductInJob(ctx, jobIds[i], "", listProducts);
        } else {
            // if the product is not acceptable for the current job, it might be outside the
            // synthesis interval

            // TODO: If the product date is after the L3A date + HalfSynthesis interval, then we can
            // mark the job finished
            //      ==> Is this enough as condition to finish the composition?
            //      ==> If we can take into account also the current time + offset (days), how much
            //      should be that offset?
        }
    }
    // TODO: we must get the last created L3A product that fits the half synthesis (is inside the
    // synthesi interval) and job's L3A product date
    //      ==> The intermediate L3A products should be saved in DB or they should be kept in the
    //      job's temporary repository?
    //      ==> If not kept in the DB, then we should get the last created L3A product
    //      ==> Anyway, the L3A product should be specific to a job as an L3A intermediate file
    //      could be created with other parameters

    // TODO: After inserting a new product, we should delete the old L3A products
    // TODO: Maybe some internal status params should be added in DB!!!
    //      Additionally, keep the original event parameters that generated job creation!!!

    // PROBLEM: How to handle the case when a processing is in progress for one job and we receive a
    // new accepted
    // product with a date later than the current processing one? In this case, the processing
    // should be delayed until
    // the finishing of the current one.
    //      ==> Does the context handles the submitted steps sequentially???
    //      ==> What happens if we receive products with an older date than the current processed
    //      ones???
}

void CompositeHandler::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList,
                                                 int nbProducts)
{
    // just create the tasks but with no information so far
    for (int i = 0; i < nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{ "composite-mask-handler", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-preprocessing", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-weigh-aot", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-weigh-on-clouds", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-total-weight", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-update-synthesis", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-splitter", {} });
    }

    // now fill the tasks hierarchy infos
    int i;
    for (i = 0; i < nbProducts; i++) {
        if (i > 0) {
            // update the mask handler with the reference of the previous composite splitter
            int nMaskHandlerIdx = i * TasksNoPerProduct;
            int nPrevCompositeSplitterIdx = (i - 1) * TasksNoPerProduct + (TasksNoPerProduct - 1);
            outAllTasksList[nMaskHandlerIdx].parentTasks.append(
                outAllTasksList[nPrevCompositeSplitterIdx]);
        }
        // the others comme naturally updated
        // composite-preprocessing -> mask-handler
        outAllTasksList[i * TasksNoPerProduct + 1].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct]);
        // weigh-aot -> composite-preprocessing
        outAllTasksList[i * TasksNoPerProduct + 2].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 1]);
        // weigh-on-clouds -> composite-preprocessing
        outAllTasksList[i * TasksNoPerProduct + 3].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 1]);
        // total-weight -> weigh-aot and weigh-on-clouds
        outAllTasksList[i * TasksNoPerProduct + 4].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 2]);
        outAllTasksList[i * TasksNoPerProduct + 4].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 3]);
        // update-synthesis -> total-weight
        outAllTasksList[i * TasksNoPerProduct + 5].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 4]);
        // composite-splitter -> update-synthesis
        outAllTasksList[i * TasksNoPerProduct + 6].parentTasks.append(
            outAllTasksList[i * TasksNoPerProduct + 5]);
    }
    // product-formatter -> the last composite-splitter
    outProdFormatterParentsList.append(outAllTasksList[outAllTasksList.size() - 1]);
}

CompositeGlobalExecutionInfos CompositeHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event,
                                             const QStringList &listProducts)
{
    int jobId = event.jobId;
    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters =
        ctx.GetJobConfigurationParameters(jobId, "processor.l3a.");

    // Get L3A Synthesis date
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
    auto synthalf = parameters["half_synthesis"].toString();

    int resolution = parameters["resolution"].toInt();
    if(resolution == 0) resolution = 10;    // TODO: We should configure the default resolution in DB

    // Get the parameters from the configuration
    // Get the Half Synthesis interval value if it was not specified by the user
    if(synthalf.length() == 0) {
        synthalf = configParameters["processor.l3a.half_synthesis"];
        if(synthalf.length() == 0) {
            synthalf = "15";
        }
    }
    auto bandsMapping = configParameters["processor.l3a.bandsmapping"];
    const auto &scatCoeffs = configParameters["processor.l3a.preproc.scatcoeffs"];

    const auto &weightAOTMin = configParameters["processor.l3a.weight.aot.minweight"];
    const auto &weightAOTMax = configParameters["processor.l3a.weight.aot.maxweight"];
    const auto &AOTMax = configParameters["processor.l3a.weight.aot.maxaot"];

    const auto &coarseRes = configParameters["processor.l3a.weight.cloud.coarseresolution"];
    const auto &sigmaSmallCloud = configParameters["processor.l3a.weight.cloud.sigmasmall"];
    const auto &sigmaLargeCloud = configParameters["processor.l3a.weight.cloud.sigmalarge"];

    const auto &weightDateMin = configParameters["processor.l3a.weight.total.weightdatemin"];

    bandsMapping = DeductBandsMappingFile(listProducts, bandsMapping, resolution);

    const auto &resolutionStr = QString::number(resolution);

    CompositeGlobalExecutionInfos globalExecInfos;
    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;
    QList<std::reference_wrapper<const TaskToSubmit>> &prodFormParTsksList = globalExecInfos.prodFormatParams.parentsTasksRef;
    CreateTasksForNewProducts(allTasksList, prodFormParTsksList, listProducts.size());

    NewStepList &steps = globalExecInfos.allStepsList;

    QString prevL3AProdRefls;
    QString prevL3AProdWeights;
    QString prevL3AProdFlags;
    QString prevL3AProdDates;
    QString prevL3ARgbFile;

    for (int i = 0; i < listProducts.size(); i++) {
        const auto &inputProduct = listProducts[i];

        TaskToSubmit &maskHandler = allTasksList[i * TasksNoPerProduct];
        TaskToSubmit &compositePreprocessing = allTasksList[i * TasksNoPerProduct + 1];
        TaskToSubmit &weightAot = allTasksList[i * TasksNoPerProduct + 2];
        TaskToSubmit &weightOnClouds = allTasksList[i * TasksNoPerProduct + 3];
        TaskToSubmit &totalWeight = allTasksList[i * TasksNoPerProduct + 4];
        TaskToSubmit &updateSynthesis = allTasksList[i * TasksNoPerProduct + 5];
        TaskToSubmit &compositeSplitter = allTasksList[i * TasksNoPerProduct + 6];

        ctx.SubmitTasks(jobId, { maskHandler, compositePreprocessing, weightAot, weightOnClouds,
                                 totalWeight, updateSynthesis, compositeSplitter });

        const auto &masksFile = maskHandler.GetFilePath("all_masks_file.tif");
        const auto &outResImgBands = compositePreprocessing.GetFilePath("img_res_bands.tif");
        const auto &cldResImg = compositePreprocessing.GetFilePath("cld_res.tif");
        const auto &waterResImg = compositePreprocessing.GetFilePath("water_res.tif");
        const auto &snowResImg = compositePreprocessing.GetFilePath("snow_res.tif");
        const auto &aotResImg = compositePreprocessing.GetFilePath("aot_res.tif");

        const auto &outWeightAotFile = weightAot.GetFilePath("weight_aot.tif");

        const auto &outWeightCldFile = weightOnClouds.GetFilePath("weight_cloud.tif");

        const auto &outTotalWeighFile = totalWeight.GetFilePath("weight_total.tif");

        const auto &outL3AResultFile = updateSynthesis.GetFilePath("L3AResult.tif");

        const auto &outL3AResultReflsFile = compositeSplitter.GetFilePath("L3AResult_refls.tif");
        const auto &outL3AResultWeightsFile =
            compositeSplitter.GetFilePath("L3AResult_weights.tif");
        const auto &outL3AResultFlagsFile = compositeSplitter.GetFilePath("L3AResult_flags.tif");
        const auto &outL3AResultDatesFile = compositeSplitter.GetFilePath("L3AResult_dates.tif");
        const auto &outL3AResultRgbFile = compositeSplitter.GetFilePath("L3AResult_rgb.tif");

        QStringList maskHandlerArgs = { "MaskHandler", "-xml",         inputProduct, "-out",
                                        masksFile,     "-sentinelres", resolutionStr };
        QStringList compositePreprocessingArgs = { "CompositePreprocessing2", "-xml", inputProduct,
                                                   "-bmap", bandsMapping, "-res", resolutionStr,
                                                   "-msk", masksFile, "-outres", outResImgBands,
                                                   "-outcmres", cldResImg, "-outwmres", waterResImg,
                                                   "-outsmres", snowResImg, "-outaotres",
                                                   aotResImg };
        if(scatCoeffs.length() > 0) {
            compositePreprocessingArgs.append("-scatcoef");
            compositePreprocessingArgs.append(scatCoeffs);
        }
        QStringList weightAotArgs = { "WeightAOT",     "-xml",     inputProduct, "-in",
                                      aotResImg,       "-waotmin", weightAOTMin, "-waotmax",
                                      weightAOTMax,    "-aotmax",  AOTMax,       "-out",
                                      outWeightAotFile };
        QStringList weightOnCloudArgs = { "WeightOnClouds", "-inxml",         inputProduct,
                                          "-incldmsk",      cldResImg,        "-coarseres",
                                          coarseRes,        "-sigmasmallcld", sigmaSmallCloud,
                                          "-sigmalargecld", sigmaLargeCloud,  "-out",
                                          outWeightCldFile };

        QStringList totalWeightArgs = { "TotalWeight",    "-xml",           inputProduct,
                                        "-waotfile",      outWeightAotFile, "-wcldfile",
                                        outWeightCldFile, "-l3adate",       l3aSynthesisDate,
                                        "-halfsynthesis", synthalf,         "-wdatemin",
                                        weightDateMin,    "-out",           outTotalWeighFile };
        QStringList updateSynthesisArgs = { "UpdateSynthesis", "-in",   outResImgBands,    "-bmap",
                                            bandsMapping,      "-xml",  inputProduct,      "-csm",
                                            cldResImg,         "-wm",   waterResImg,       "-sm",
                                            snowResImg,        "-wl2a", outTotalWeighFile, "-out",
                                            outL3AResultFile };
        if (i > 0) {
            updateSynthesisArgs.append("-prevl3aw");
            updateSynthesisArgs.append(prevL3AProdWeights);
            updateSynthesisArgs.append("-prevl3ad");
            updateSynthesisArgs.append(prevL3AProdDates);
            updateSynthesisArgs.append("-prevl3ar");
            updateSynthesisArgs.append(prevL3AProdRefls);
            updateSynthesisArgs.append("-prevl3af");
            updateSynthesisArgs.append(prevL3AProdFlags);
        }

        QStringList compositeSplitterArgs = { "CompositeSplitter2",
                                              "-in", outL3AResultFile,
                                              "-xml", inputProduct,
                                              "-bmap", bandsMapping,
                                              "-outweights", outL3AResultWeightsFile,
                                              "-outdates", outL3AResultDatesFile,
                                              "-outrefls", outL3AResultReflsFile,
                                              "-outflags", outL3AResultFlagsFile,
                                              "-outrgb", outL3AResultRgbFile };

        // save the created L3A product file for the next product creation
        prevL3AProdRefls = outL3AResultReflsFile;
        prevL3AProdWeights = outL3AResultWeightsFile;
        prevL3AProdFlags = outL3AResultFlagsFile;
        prevL3AProdDates = outL3AResultDatesFile;
        prevL3ARgbFile = outL3AResultRgbFile;

        // add these steps to the steps list to be submitted
        steps.append(maskHandler.CreateStep("MaskHandler", maskHandlerArgs));
        steps.append(compositePreprocessing.CreateStep("CompositePreprocessing",
                                                       compositePreprocessingArgs));
        steps.append(weightAot.CreateStep("WeightAOT", weightAotArgs));
        steps.append(weightOnClouds.CreateStep("WeightOnClouds", weightOnCloudArgs));
        steps.append(totalWeight.CreateStep("TotalWeight", totalWeightArgs));
        steps.append(updateSynthesis.CreateStep("UpdateSynthesis", updateSynthesisArgs));
        steps.append(compositeSplitter.CreateStep("CompositeSplitter", compositeSplitterArgs));
    }

    CompositeProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.prevL3AProdRefls = prevL3AProdRefls;
    productFormatterParams.prevL3AProdWeights = prevL3AProdWeights;
    productFormatterParams.prevL3AProdFlags = prevL3AProdFlags;
    productFormatterParams.prevL3AProdDates = prevL3AProdDates;
    productFormatterParams.prevL3ARgbFile = prevL3ARgbFile;
    // Get the tile ID from the product XML name. We extract it from the first product in the list as all
    // producs should be for the same tile
    productFormatterParams.tileId = ProcessorHandlerHelper::GetTileId(listProducts);

    return globalExecInfos;
}

void CompositeHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const QJsonObject &parameters,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts)
{
    std::ofstream executionInfosFile;
    try {
        // Get L3A Synthesis date
        const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
        // Get the Half Synthesis interval value
        const auto &synthalf = parameters["half_synthesis"].toString();

        // Get the parameters from the configuration
        const auto &bandsMapping = configParameters["processor.l3a.bandsmapping"];
        const auto &scatCoeffs = configParameters["processor.l3a.preproc.scatcoeffs"];

        const auto &weightAOTMin = configParameters["processor.l3a.weight.aot.minweight"];
        const auto &weightAOTMax = configParameters["processor.l3a.weight.aot.maxweight"];
        const auto &AOTMax = configParameters["processor.l3a.weight.aot.maxaot"];

        const auto &coarseRes = configParameters["processor.l3a.weight.cloud.coarseresolution"];
        const auto &sigmaSmallCloud = configParameters["processor.l3a.weight.cloud.sigmasmall"];
        const auto &sigmaLargeCloud = configParameters["processor.l3a.weight.cloud.sigmalarge"];

        const auto &weightDateMin = configParameters["processor.l3a.weight.total.weightdatemin"];

        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "    <bands_mapping_file>" << bandsMapping.toStdString()
                           << "</bands_mapping_file>" << std::endl;
        executionInfosFile << "    <scattering_coefficients_file>" << scatCoeffs.toStdString()
                           << "</scattering_coefficients_file>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <Weight_AOT>" << std::endl;
        executionInfosFile << "    <weight_aot_min>" << weightAOTMin.toStdString()
                           << "</weight_aot_min>" << std::endl;
        executionInfosFile << "    <weight_aot_max>" << weightAOTMax.toStdString()
                           << "</weight_aot_max>" << std::endl;
        executionInfosFile << "    <aot_max>" << AOTMax.toStdString() << "</aot_max>" << std::endl;
        executionInfosFile << "  </Weight_AOT>" << std::endl;

        executionInfosFile << "  <Weight_On_Clouds>" << std::endl;
        executionInfosFile << "    <coarse_res>" << coarseRes.toStdString() << "</coarse_res>"
                           << std::endl;
        executionInfosFile << "    <sigma_small_cloud>" << sigmaSmallCloud.toStdString()
                           << "</sigma_small_cloud>" << std::endl;
        executionInfosFile << "    <sigma_large_cloud>" << sigmaLargeCloud.toStdString()
                           << "</sigma_large_cloud>" << std::endl;
        executionInfosFile << "  </Weight_On_Clouds>" << std::endl;

        executionInfosFile << "  <Weight_On_Date>" << std::endl;
        executionInfosFile << "    <weight_date_min>" << weightDateMin.toStdString()
                           << "</weight_date_min>" << std::endl;
        executionInfosFile << "    <l3a_product_date>" << l3aSynthesisDate.toStdString()
                           << "</l3a_product_date>" << std::endl;
        executionInfosFile << "    <half_synthesis>" << synthalf.toStdString()
                           << "</half_synthesis>" << std::endl;
        executionInfosFile << "  </Weight_On_Date>" << std::endl;

        executionInfosFile << "  <Dates_information>" << std::endl;
        // TODO: We should get these infos somehow but without parsing here anything
        // executionInfosFile << "    <start_date>" << 2013031 << "</start_date>" << std::endl;
        // executionInfosFile << "    <end_date>" << 20130422 << "</end_date>" << std::endl;
        executionInfosFile << "    <synthesis_date>" << l3aSynthesisDate.toStdString()
                           << "</synthesis_date>" << std::endl;
        executionInfosFile << "    <synthesis_half>" << synthalf.toStdString()
                           << "</synthesis_half>" << std::endl;
        executionInfosFile << "  </Dates_information>" << std::endl;

        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i < listProducts.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">"
                               << listProducts[i].toStdString() << "</XML_" << std::to_string(i)
                               << ">" << std::endl;
        }
        executionInfosFile << "  </XML_files>" << std::endl;
        executionInfosFile << "</metadata>" << std::endl;
        executionInfosFile.close();
    } catch (...) {
    }
}

// This function removes the files from the list that are outside the synthesis interval
// and that should not be used in the composition
void CompositeHandler::FilterInputProducts(QStringList &listFiles,
                                           int productDate,
                                           int halfSynthesis)
{
    // TODO: we should extract here the date of the product to compare it with tha synthesis
    // interval
    Q_UNUSED(listFiles);
    Q_UNUSED(productDate);
    Q_UNUSED(halfSynthesis);
}

void CompositeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QStringList listProducts = GetL2AInputProducts(ctx, event);
    if(listProducts.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        return;
    }

    QMap<QString, QStringList> mapTiles = ProcessorHandlerHelper::GroupTiles(listProducts);
    QList<CompositeProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    for(auto tile : mapTiles.keys())
    {
       QStringList listTemporalTiles = mapTiles.value(tile);
       CompositeGlobalExecutionInfos infos = HandleNewTilesList(ctx, event, listTemporalTiles);
       listParams.append(infos.prodFormatParams);
       productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
       allTasksList.append(infos.allTasksList);
       allSteps.append(infos.allStepsList);
    }

    ctx.SubmitTasks(event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}

void CompositeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);

        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "" && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::L3AProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate, prodName, quicklook,
                                footPrint, TileList() });
            // Now remove the job folder containing temporary files
            // TODO: Reinsert this line - commented only for debug purposes
            //RemoveJobFolder(ctx, event.jobId);
        }
    }
}

QStringList CompositeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<CompositeProductFormatterParams> &productParams) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3a.lai.");

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();

    WriteExecutionInfosFile(executionInfosPath, parameters, configParameters, listProducts);

    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L3A",
                                         "-timeperiod", l3aSynthesisDate,
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-processor", "composite",
                                         "-gipp", executionInfosPath,
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts[listProducts.size() - 1];

    productFormatterArgs += "-processor.composite.refls";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.prevL3AProdRefls;
    }
    productFormatterArgs += "-processor.composite.weights";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.prevL3AProdWeights;
    }
    productFormatterArgs += "-processor.composite.flags";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.prevL3AProdFlags;
    }
    productFormatterArgs += "-processor.composite.dates";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.prevL3AProdDates;
    }
    productFormatterArgs += "-processor.composite.rgb";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.prevL3ARgbFile;
    }

    return productFormatterArgs;
}


bool CompositeHandler::IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event)
{
    Q_UNUSED(jobId);
    Q_UNUSED(event);

    return false;
}

QStringList CompositeHandler::GetMissionsFromBandsMapping(const QString &bandsMappingFile) {
    // Normally, this is a small file
    QStringList listLines = ProcessorHandlerHelper::GetTextFileLines(bandsMappingFile);
    if(listLines.size() > 0) {
        // we get the first line only
        QString firstLine = listLines[0];
        return firstLine.split(",");
    }
    return QStringList();
}

QString CompositeHandler::DeductBandsMappingFile(const QStringList &listProducts,
                                                 const QString &bandsMappingFile, int &resolution) {
    QFileInfo fileInfo(bandsMappingFile);

    // by default, we consider this is a dir
    QString curBandsMappingPath = bandsMappingFile;
    if(!fileInfo.isDir())
        curBandsMappingPath = fileInfo.dir().absolutePath();
    QStringList listUniqueProductTypes;
    for (int i = 0; i < listProducts.size(); i++) {
        QString productType = ProcessorHandlerHelper::GetL2AProductTypeFromTile(listProducts[i]);
        if(!listUniqueProductTypes.contains(productType)) {
            listUniqueProductTypes.append(productType);
        }
    }
    int cntUniqueProdTypes = listUniqueProductTypes.size();
    if(cntUniqueProdTypes < 1 || cntUniqueProdTypes > 2 ) {
        return bandsMappingFile;
    }
    if(cntUniqueProdTypes == 1) {
        if(listUniqueProductTypes[0] == "SENTINEL") {
            return (curBandsMappingPath + "/bands_mapping_s2.txt");
        }
        if(listUniqueProductTypes[0] == "LANDSAT_8") {
            resolution = 30;
            return (curBandsMappingPath + "/bands_mapping_L8.txt");
        }
        if(listUniqueProductTypes[0] == "SPOT4") {
            resolution = 20;
            return (curBandsMappingPath + "/bands_mapping_spot.txt");
        }
        if(listUniqueProductTypes[0] == "SPOT5") {
            resolution = 10;
            return (curBandsMappingPath + "/bands_mapping_spot5.txt");
        }
    } else {
        if(listUniqueProductTypes.contains("SPOT4") && listUniqueProductTypes.contains("LANDSAT_8")) {
            resolution = 10;
            return (curBandsMappingPath + "/bands_mapping_Spot4_L8.txt");
        }
    }
    return bandsMappingFile;
}


ProcessorJobDefinitionParams CompositeHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3a."), siteId, requestOverrideCfgValues);

    ProcessorJobDefinitionParams params;
    params.isValid = false;

    int halfSynthesis = mapCfg["processor.l3a.half_synthesis"].value.toInt();
    if(halfSynthesis == 0)
        halfSynthesis = 15;
    int synthDateOffset = mapCfg["processor.l3a.synth_date_sched_offset"].value.toInt();
    if(synthDateOffset == 0)
        synthDateOffset = 30;

    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    // compute the half synthesis date
    QDateTime halfSynthesisDate = qScheduledDate.addDays(-synthDateOffset);
    // compute the start and date time
    QDateTime startDate = halfSynthesisDate.addDays(-halfSynthesis);
    QDateTime endDate = halfSynthesisDate.addDays(halfSynthesis);

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // we need at least 1 product available in order to be able to create a L3A product
    if(params.productList.size() > 0) {
        params.isValid = true;
        params.jsonParameters = "{ \"synthesis_date\": \"" + halfSynthesisDate.toString("yyyymmdd") + "\"}";
    }
    //return QString("Cannot execute Composite processor. There should be at least 4 products but we have only %1 L2A products available!").arg(usedProductList.size());
    return params;
}


