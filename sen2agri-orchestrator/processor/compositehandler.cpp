#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "compositehandler.hpp"

#define TasksNoPerProduct 7

void CompositeHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                            const ProductAvailableEvent &event)
{
    // TODO: Here we must get all job IDs for the current processor id
    std::vector<int> jobIds;

    // for each job ID, check if the job accepts the current product
    for(unsigned int i = 0; i<jobIds.size(); i++) {
        // Check if the product can be processed by the current job
        if(IsProductAcceptableForJob(jobIds[i], event)) {
            // create a list with products that will have in this case only one element
            QStringList listProducts;
            // Get the product path from the productId
            QString strProductPath ; // TODO = ctx.GetProductPathFromId(event.productId);
            listProducts.append(strProductPath);
            // process the received L2A product in the current job
            HandleNewProductInJob(ctx, jobIds[i], "", listProducts);
        } else {
            // if the product is not acceptable for the current job, it might be outside the
            // synthesis interval

            // TODO: If the product date is after the L3A date + HalfSynthesis interval, then we can mark the job finished
            //      ==> Is this enough as condition to finish the composition?
            //      ==> If we can take into account also the current time + offset (days), how much should be that offset?
        }
    }
    // TODO: we must get the last created L3A product that fits the half synthesis (is inside the synthesi interval) and job's L3A product date
    //      ==> The intermediate L3A products should be saved in DB or they should be kept in the job's temporary repository?
    //      ==> If not kept in the DB, then we should get the last created L3A product
    //      ==> Anyway, the L3A product should be specific to a job as an L3A intermediate file could be created with other parameters

    // TODO: After inserting a new product, we should delete the old L3A products
    // TODO: Maybe some internal status params should be added in DB!!!
    //      Additionally, keep the original event parameters that generated job creation!!!

    // PROBLEM: How to handle the case when a processing is in progress for one job and we receive a new accepted
    // product with a date later than the current processing one? In this case, the processing should be delayed until
    // the finishing of the current one.
    //      ==> Does the context handles the submitted steps sequentially???
    //      ==> What happens if we receive products with an older date than the current processed ones???
}

void CompositeHandler::CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts) {
    // just create the tasks but with no information so far
    for(int i = 0; i<nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{"composite-mask-handler", {}});
        outAllTasksList.append(TaskToSubmit{"composite-preprocessing", {}});
        outAllTasksList.append(TaskToSubmit{"composite-weigh-aot", {}});
        outAllTasksList.append(TaskToSubmit{"composite-weigh-on-clouds", {}});
        outAllTasksList.append(TaskToSubmit{"composite-total-weight", {}});
        outAllTasksList.append(TaskToSubmit{"composite-update-synthesis", {}});
        outAllTasksList.append(TaskToSubmit{"composite-splitter", {}});
    }
    // The product formatter task will be at the end and only once (not for each product)
    outAllTasksList.append(TaskToSubmit{"product-formatter", {}});

    // now fill the tasks hierarchy infos
    int i;
    for(i = 0; i<nbProducts; i++) {
        if(i > 0) {
            // update the mask handler with the reference of the previous composite splitter
            int nMaskHandlerIdx = i*TasksNoPerProduct;
            int nPrevCompositeSplitterIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
            outAllTasksList[nMaskHandlerIdx].parentTasks.append(outAllTasksList[nPrevCompositeSplitterIdx]);
        }
        // the others comme naturally updated
        // composite-preprocessing -> mask-handler
        outAllTasksList[i*TasksNoPerProduct+1].parentTasks.append(outAllTasksList[i*TasksNoPerProduct]);
        // weigh-aot -> composite-preprocessing
        outAllTasksList[i*TasksNoPerProduct+2].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
        //weigh-on-clouds -> composite-preprocessing
        outAllTasksList[i*TasksNoPerProduct+3].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
        //total-weight -> weigh-aot and weigh-on-clouds
        outAllTasksList[i*TasksNoPerProduct+4].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+2]);
        outAllTasksList[i*TasksNoPerProduct+4].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+3]);
        //update-synthesis -> total-weight
        outAllTasksList[i*TasksNoPerProduct+5].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+4]);
        //composite-splitter -> update-synthesis
        outAllTasksList[i*TasksNoPerProduct+6].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+5]);
    }
    //product-formatter -> the last composite-splitter
    outAllTasksList[outAllTasksList.size()-1].parentTasks.append(outAllTasksList[outAllTasksList.size()-2]);
}

void CompositeHandler::HandleNewProductInJob(EventProcessingContext &ctx, int jobId, const QString &jsonParams, const QStringList &listProducts) {

    const QJsonObject &parameters = QJsonDocument::fromJson(jsonParams.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(jobId, "processor.l3a.");

    // Get L3A Synthesis date
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
    // Get the Half Synthesis interval value
    const auto &synthalf = parameters["half_synthesis"].toString();
    const auto &resolution = QString::number(parameters["resolution"].toInt());

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

    QList<TaskToSubmit> allTasksList;
    CreateNewProductInJobTasks(allTasksList, listProducts.size());

    NewStepList steps;
    QString prevL3AProdRefls;
    QString prevL3AProdWeights;
    QString prevL3AProdFlags;
    QString prevL3AProdDates;
    QString prevL3ARgbFile;

    // the product formatter is the last in the task list
    TaskToSubmit &productFormatter = allTasksList[allTasksList.size()-1];

    for (int i = 0; i<listProducts.size(); i++) {
        const auto &inputProduct = listProducts[i];

        TaskToSubmit &maskHandler = allTasksList[i*TasksNoPerProduct];
        TaskToSubmit &compositePreprocessing = allTasksList[i*TasksNoPerProduct+1];
        TaskToSubmit &weightAot = allTasksList[i*TasksNoPerProduct+2];
        TaskToSubmit &weightOnClouds = allTasksList[i*TasksNoPerProduct+3];
        TaskToSubmit &totalWeight = allTasksList[i*TasksNoPerProduct+4];
        TaskToSubmit &updateSynthesis = allTasksList[i*TasksNoPerProduct+5];
        TaskToSubmit &compositeSplitter = allTasksList[i*TasksNoPerProduct+6];

        ctx.SubmitTasks(jobId, { maskHandler, compositePreprocessing, weightAot, weightOnClouds,
                                 totalWeight, updateSynthesis, compositeSplitter
        });

        const auto & masksFile = maskHandler.GetFilePath("all_masks_file.tif");
        const auto & outResImgBands = compositePreprocessing.GetFilePath("img_res_bands.tif");
        const auto & cldResImg = compositePreprocessing.GetFilePath("cld_res.tif");
        const auto & waterResImg = compositePreprocessing.GetFilePath("water_res.tif");
        const auto & snowResImg = compositePreprocessing.GetFilePath("snow_res.tif");
        const auto & aotResImg = compositePreprocessing.GetFilePath("aot_res.tif");

        const auto & outWeightAotFile = weightAot.GetFilePath("weight_aot.tif");

        const auto & outWeightCldFile = weightOnClouds.GetFilePath("weight_cloud.tif");

        const auto & outTotalWeighFile = totalWeight.GetFilePath("weight_total.tif");

        const auto & outL3AResultFile = updateSynthesis.GetFilePath("L3AResult.tif");

        const auto & outL3AResultReflsFile = compositeSplitter.GetFilePath("L3AResult_refls.tif");
        const auto & outL3AResultWeightsFile = compositeSplitter.GetFilePath("L3AResult_weights.tif");
        const auto & outL3AResultFlagsFile = compositeSplitter.GetFilePath("L3AResult_flags.tif");
        const auto & outL3AResultDatesFile = compositeSplitter.GetFilePath("L3AResult_dates.tif");
        const auto & outL3AResultRgbFile = compositeSplitter.GetFilePath("L3AResult_rgb.tif");

        QStringList maskHandlerArgs = { "MaskHandler",
                                           "-xml", inputProduct,
                                           "-out", masksFile,
                                           "-sentinelres", resolution};
        QStringList compositePreprocessingArgs = { "CompositePreprocessing2",
                                            "-xml", inputProduct,
                                            "-bmap", bandsMapping,
                                            "-res", resolution,
                                            //"-scatcoef", scatCoeffs,  //TODO:
                                            "-msk", masksFile,
                                            "-outres", outResImgBands,
                                            "-outcmres", cldResImg,
                                            "-outwmres", waterResImg,
                                            "-outsmres", snowResImg,
                                            "-outaotres", aotResImg
                                      };
        QStringList weightAotArgs = { "WeightAOT",
                                        "-xml", inputProduct,
                                        "-in", aotResImg,
                                        "-waotmin", weightAOTMin,
                                        "-waotmax", weightAOTMax,
                                        "-aotmax", AOTMax,
                                        "-out", outWeightAotFile
                                    };
        QStringList weightOnCloudArgs = { "WeightOnClouds",
                                        "-incldmsk", cldResImg,
                                        "-coarseres", coarseRes,
                                        "-sigmasmallcld", sigmaSmallCloud,
                                        "-sigmalargecld", sigmaLargeCloud,
                                        "-out", outWeightCldFile
                                    };

        QStringList totalWeightArgs = { "TotalWeight",
                                        "-xml", inputProduct,
                                        "-waotfile", outWeightAotFile,
                                        "-wcldfile", outWeightCldFile,
                                        "-l3adate", l3aSynthesisDate,
                                        "-halfsynthesis", synthalf,
                                        "-wdatemin", weightDateMin,
                                        "-out", outTotalWeighFile
                                    };
        QStringList updateSynthesisArgs = { "UpdateSynthesis",
                                            "-in", outResImgBands,
                                            "-bmap", bandsMapping,
                                            "-xml", inputProduct,
                                            "-csm", cldResImg,
                                            "-wm", waterResImg,
                                            "-sm", snowResImg,
                                            "-wl2a", outTotalWeighFile,
                                            "-out", outL3AResultFile
                                        };
        if(i > 0) {
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
                                        "-outrgb", outL3AResultRgbFile
                                    };

        // save the created L3A product file for the next product creation
        prevL3AProdRefls = outL3AResultReflsFile;
        prevL3AProdWeights = outL3AResultWeightsFile;
        prevL3AProdFlags = outL3AResultFlagsFile;
        prevL3AProdDates = outL3AResultDatesFile;
        prevL3ARgbFile = outL3AResultRgbFile;

        // add these steps to the steps list to be submitted
        steps.append(maskHandler.CreateStep("MaskHandler", maskHandlerArgs));
        steps.append(compositePreprocessing.CreateStep("CompositePreprocessing", compositePreprocessingArgs));
        steps.append(weightAot.CreateStep("WeightAOT", weightAotArgs));
        steps.append(weightOnClouds.CreateStep("WeightOnClouds", weightOnCloudArgs));
        steps.append(totalWeight.CreateStep("TotalWeight", totalWeightArgs));
        steps.append(updateSynthesis.CreateStep("UpdateSynthesis", updateSynthesisArgs));
        steps.append(compositeSplitter.CreateStep("CompositeSplitter", compositeSplitterArgs));
    }

    // submit the product formatter task
    ctx.SubmitTasks(jobId, { productFormatter });

    const auto &targetFolder = productFormatter.GetFilePath("");
    const auto &executionInfosPath = productFormatter.GetFilePath("executionInfos.txt");

    WriteExecutionInfosFile(executionInfosPath, parameters, configParameters, listProducts);
    QStringList productFormatterArgs = { "ProductFormatter",
                                    "-destroot", targetFolder,
                                    "-fileclass", "SVT1",
                                    "-level", "L3A",
                                    "-timeperiod", l3aSynthesisDate,
                                    "-baseline", "01.00",
                                    "-processor", "composite",
                                    "-processor.composite.refls", "TILE_none", prevL3AProdRefls,
                                    "-processor.composite.weights", "TILE_none", prevL3AProdWeights,
                                    "-processor.composite.flags", "TILE_none", prevL3AProdFlags,
                                    "-processor.composite.dates", "TILE_none", prevL3AProdDates,
                                    "-processor.composite.rgb", "TILE_none", prevL3ARgbFile,
                                    "-gipp", executionInfosPath,
                                    "-il", listProducts[listProducts.size()-1]
                                };

    steps.append(productFormatter.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);
}

void CompositeHandler::WriteExecutionInfosFile(const QString &executionInfosPath, const QJsonObject &parameters,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
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
        executionInfosFile << "    <bands_mapping_file>" << bandsMapping.toStdString() << "</bands_mapping_file>" << std::endl;
        executionInfosFile << "    <scattering_coefficients_file>"<< scatCoeffs.toStdString() <<"</scattering_coefficients_file>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <Weight_AOT>" << std::endl;
        executionInfosFile << "    <weight_aot_min>" << weightAOTMin.toStdString() << "</weight_aot_min>" << std::endl;
        executionInfosFile << "    <weight_aot_max>"<< weightAOTMax.toStdString() <<"</weight_aot_max>" << std::endl;
        executionInfosFile << "    <aot_max>" << AOTMax.toStdString() << "</aot_max>" << std::endl;
        executionInfosFile << "  </Weight_AOT>" << std::endl;

        executionInfosFile << "  <Weight_On_Clouds>" << std::endl;
        executionInfosFile << "    <coarse_res>" << coarseRes.toStdString() << "</coarse_res>" << std::endl;
        executionInfosFile << "    <sigma_small_cloud>" << sigmaSmallCloud.toStdString() << "</sigma_small_cloud>" << std::endl;
        executionInfosFile << "    <sigma_large_cloud>" << sigmaLargeCloud.toStdString() << "</sigma_large_cloud>" << std::endl;
        executionInfosFile << "  </Weight_On_Clouds>" << std::endl;

        executionInfosFile << "  <Weight_On_Date>" << std::endl;
        executionInfosFile << "    <weight_date_min>" <<weightDateMin.toStdString() << "</weight_date_min>" << std::endl;
        executionInfosFile << "    <l3a_product_date>" << l3aSynthesisDate.toStdString() << "</l3a_product_date>" << std::endl;
        executionInfosFile << "    <half_synthesis>" << synthalf.toStdString() << "</half_synthesis>" << std::endl;
        executionInfosFile << "  </Weight_On_Date>" << std::endl;

        executionInfosFile << "  <Dates_information>" << std::endl;
        // TODO: We should get these infos somehow but without parsing here anything
        //executionInfosFile << "    <start_date>" << 2013031 << "</start_date>" << std::endl;
        //executionInfosFile << "    <end_date>" << 20130422 << "</end_date>" << std::endl;
        executionInfosFile << "    <synthesis_date>" << l3aSynthesisDate.toStdString() << "</synthesis_date>" << std::endl;
        executionInfosFile << "    <synthesis_half>" << synthalf.toStdString() << "</synthesis_half>" << std::endl;
        executionInfosFile << "  </Dates_information>" << std::endl;

        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i<listProducts.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">" << listProducts[i].toStdString()
                               << "</XML_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "  </XML_files>" << std::endl;
        executionInfosFile << "</metadata>" << std::endl;
        executionInfosFile.close();
    }
    catch(...)
    {

    }
}

// This function removes the files from the list that are outside the synthesis interval
// and that should not be used in the composition
void CompositeHandler::FilterInputProducts(QStringList &listFiles, int productDate, int halfSynthesis)
{
    // TODO: we should extract here the date of the product to compare it with tha synthesis interval
    Q_UNUSED(listFiles);
    Q_UNUSED(productDate);
    Q_UNUSED(halfSynthesis);
}

void CompositeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();

    QStringList listProducts;
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFile(inputProduct.toString()));
    }
    // process the received L2A products in the current job
    HandleNewProductInJob(ctx, event.jobId, event.parametersJson, listProducts);
}

void CompositeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "compute-confusion-matrix") {
        const auto &outputs = ctx.GetTaskConsoleOutputs(event.taskId);
        for (const auto &output : outputs) {
            QRegularExpression reK("Kappa index: (\\d+(?:\\.\\d*)?)");
            QRegularExpression reAcc("Overall accuracy index: (\\d+(?:\\.\\d*)?)");

            const auto &mK = reK.match(output.stdOutText);
            const auto &mAcc = reAcc.match(output.stdOutText);

            const auto &statisticsPath =
                ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "statistics.txt";
            QFile file(statisticsPath);
            if (!file.open(QIODevice::WriteOnly)) {
                throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
                                             .arg(statisticsPath)
                                             .arg(file.errorString())
                                             .toStdString());
            }

            QTextStream s(&file);
            s << mK.captured(1) << ' ' << mAcc.captured(1);
        }

        ctx.MarkJobFinished(event.jobId);
    }

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}


bool CompositeHandler::IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event) {
    Q_UNUSED(jobId);
    Q_UNUSED(event);

    return false;
}
