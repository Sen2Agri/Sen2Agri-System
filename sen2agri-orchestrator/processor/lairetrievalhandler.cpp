#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler.hpp"

#define TasksNoPerProduct 4

void LaiRetrievalHandler::CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    for(int i = 0; i<nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-models-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
    }
    outAllTasksList.append(TaskToSubmit{"lai-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-err-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-local-window-reprocessing", {}});
    outAllTasksList.append(TaskToSubmit{"lai-local-window-reproc-splitter", {}});
    outAllTasksList.append(TaskToSubmit{"lai-fitted-reprocessing", {}});
    outAllTasksList.append(TaskToSubmit{"lai-fitted-reproc-splitter", {}});
    outAllTasksList.append(TaskToSubmit{"lai-product-formatter", {}});

    // now fill the tasks hierarchy infos

    //   ----------------------------- LOOP ---------------------------------
    //   |                                                                  |
    //   |                      ndvi-rvi-extraction                         |
    //   |                              |                                   |
    //   |                      get-lai-retrieval-model                     |
    //   |                              |                                   |
    //   |              -----------------------------------------           |
    //   |              |                                       |           |
    //   |      bv-image-inversion                 bv-err-image-inversion   |
    //   |              |                                       |           |
    //   |              -----------------------------------------           |
    //   |                              |                                   |
    //   --------------------------------------------------------------------
    //                                  |
    //              ---------------------------------------------
    //              |                                           |
    //      time-series-builder                     err-time-series-builder
    //              |                                           |
    //              ---------------------------------------------
    //                                  |
    //              ---------------------------------------------
    //              |                                           |
    //      profile-reprocessing                fitted-profile-reprocessing
    //              |                                           |
    //      reprocessed-profile-splitter        fitted-reprocessed-profile-splitter
    //              |                                           |
    //              ---------------------------------------------
    //                                  |
    //                          product-formatter
    //
    int i;
    for(i = 0; i<nbProducts; i++) {
        if(i > 0) {
            // update the ndvi-rvi-extractor with the reference of the previous bv-err-image-inversion
            int nNdviRviExtractorIdx = i*TasksNoPerProduct;
            int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
            outAllTasksList[nNdviRviExtractorIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
            outAllTasksList[nNdviRviExtractorIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);
        }
        // the others comme naturally updated
        // get-lai-retrieval-model -> ndvi-rvi-extraction
        outAllTasksList[i*TasksNoPerProduct+1].parentTasks.append(outAllTasksList[i*TasksNoPerProduct]);
        // bv-image-inversion -> get-lai-retrieval-model
        outAllTasksList[i*TasksNoPerProduct+2].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
        // bv-err-image-inversion -> get-lai-retrieval-model
        outAllTasksList[i*TasksNoPerProduct+3].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
    }

    // time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
    int nCurIdx = i*TasksNoPerProduct;
    int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
    outAllTasksList[nCurIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
    outAllTasksList[nCurIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    //err-time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
    outAllTasksList[nCurIdx+1].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
    outAllTasksList[nCurIdx+1].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    //profile-reprocessing -> time-series-builder AND err-time-series-builder
    outAllTasksList[nCurIdx+2].parentTasks.append(outAllTasksList[nCurIdx]);
    outAllTasksList[nCurIdx+2].parentTasks.append(outAllTasksList[nCurIdx+1]);

    //reprocessed-profile-splitter -> profile-reprocessing
    outAllTasksList[nCurIdx+3].parentTasks.append(outAllTasksList[nCurIdx+2]);

    //fitted-profile-reprocessing -> time-series-builder AND err-time-series-builder
    outAllTasksList[nCurIdx+4].parentTasks.append(outAllTasksList[nCurIdx]);
    outAllTasksList[nCurIdx+4].parentTasks.append(outAllTasksList[nCurIdx+1]);

    //fitted-reprocessed-profile-splitter -> fitted-profile-reprocessing
    outAllTasksList[nCurIdx+5].parentTasks.append(outAllTasksList[nCurIdx+4]);

    //product-formatter -> reprocessed-profile-splitter AND fitted-reprocessed-profile-splitter
    outAllTasksList[nCurIdx+6].parentTasks.append(outAllTasksList[nCurIdx+3]);
    outAllTasksList[nCurIdx+6].parentTasks.append(outAllTasksList[nCurIdx+5]);
}

void LaiRetrievalHandler::HandleNewProductInJob(EventProcessingContext &ctx, int jobId,
                                                const QString &jsonParams, const QStringList &listProducts) {

    const QJsonObject &parameters = QJsonDocument::fromJson(jsonParams.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(jobId, "processor.l3b.lai.");

    // Get L3A Synthesis date
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
    // Get the Half Synthesis interval value
    const auto &resolution = QString::number(parameters["resolution"].toInt());

    QList<TaskToSubmit> allTasksList;
    CreateNewProductInJobTasks(allTasksList, listProducts.size());

    NewStepList steps;
    QStringList monoDateLaiFileNames;
    QStringList monoDateErrLaiFileNames;

    // first extract the model file names from the models folder
    QStringList modelsList;
    QStringList errModelsList;
    GetModelFileList(modelsList, "Model_*.txt", configParameters);
    GetModelFileList(errModelsList, "Err_Est_Model_*.txt", configParameters);

    int i;
    for (i = 0; i<listProducts.size(); i++) {
        const auto &inputProduct = listProducts[i];

        TaskToSubmit &ndviRviExtractorTask = allTasksList[i*TasksNoPerProduct];
        TaskToSubmit &getLaiRetrievalModelTask = allTasksList[i*TasksNoPerProduct+1];
        TaskToSubmit &bvImageInversionTask = allTasksList[i*TasksNoPerProduct+2];
        TaskToSubmit &bvErrImageInversionTask = allTasksList[i*TasksNoPerProduct+3];

        ctx.SubmitTasks(jobId, { ndviRviExtractorTask,
                                 getLaiRetrievalModelTask,
                                 bvImageInversionTask,
                                 bvErrImageInversionTask
        });

        const auto & ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
        const auto & modelFileName = getLaiRetrievalModelTask.GetFilePath("model_file.txt");
        const auto & errModelFileName = getLaiRetrievalModelTask.GetFilePath("err_model_file.txt");
        const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_img.tif");
        const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_err_img.tif");

        // save the mono date LAI file name list
        monoDateLaiFileNames.append(monoDateLaiFileName);
        monoDateErrLaiFileNames.append(monoDateErrFileName);

        QStringList ndviRviExtractionArgs = { "NdviRviExtraction2",
                                           "-xml", inputProduct,
                                           "-fts", ftsFile,
                                           "-outres", resolution
                                            };
        QStringList getLaiModelArgs = { "GetLaiRetrievalModel",
                                            "-xml", inputProduct,
                                            "-ilmodels", modelsList.join(" "),
                                            "-ilerrmodels", errModelsList.join(" "),
                                            "-outm", modelFileName,
                                            "-outerr", errModelFileName
                                      };
        QStringList bvImageInvArgs = { "BVImageInversion",
                                        "-in", ftsFile,
                                        "-modelfile", modelFileName,
                                        "-out", monoDateLaiFileName
                                    };
        QStringList bvErrImageInvArgs = { "BVImageInversion",
                                          "-in", ftsFile,
                                          "-modelfile", errModelFileName,
                                          "-out", monoDateErrFileName
                                      };
        // add these steps to the steps list to be submitted
        steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));
        steps.append(getLaiRetrievalModelTask.CreateStep("GetLaiRetrievalModel", getLaiModelArgs));
        steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
        steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
    }

    int nCurIdx = i*TasksNoPerProduct;
    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[nCurIdx];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[nCurIdx+1];
    TaskToSubmit &profileReprocTask = allTasksList[nCurIdx+2];
    TaskToSubmit &profileReprocSplitTask = allTasksList[nCurIdx+3];
    TaskToSubmit &fittedProfileReprocTask = allTasksList[nCurIdx+4];
    TaskToSubmit &fittedProfileReprocSplitTask = allTasksList[nCurIdx+5];
    TaskToSubmit &productFormatterTask = allTasksList[nCurIdx+6];

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");

    const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
    const auto & fittedTimeSeriesFileName = fittedProfileReprocTask.GetFilePath("FittedTimeSeries.tif");

    const auto & reprocFileListFileName = profileReprocSplitTask.GetFilePath("FittedFilesList.txt");
    const auto & fittedFileListFileName = fittedProfileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");

    ctx.SubmitTasks(jobId, { imgTimeSeriesBuilderTask,
                             errTimeSeriesBuilderTask,
                             profileReprocTask,
                             profileReprocSplitTask,
                             fittedProfileReprocTask,
                             fittedProfileReprocSplitTask,
                             productFormatterTask
    });

    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
                                    "-il", monoDateLaiFileNames.join(" "),
                                    "-out", allLaiTimeSeriesFileName
                                };
    QStringList errTimeSeriesBuilderArgs = { "TimeSeriesBuilder",
                                        "-il", monoDateErrLaiFileNames.join(" "),
                                        "-out", allErrTimeSeriesFileName
                                    };

    const auto &localWindowBwr = configParameters["processor.l3b.lai.localwnd.bwr"];
    const auto &localWindowFwr = configParameters["processor.l3b.lai.localwnd.fwr"];
    QStringList profileReprocessingArgs = { "ProfileReprocessing",
                                            "-lai", allLaiTimeSeriesFileName,
                                            "-err", allErrTimeSeriesFileName,
                                            "-ilxml", listProducts.join(" "),
                                            "-opf", reprocTimeSeriesFileName,
                                            "-algo", "local",
                                            "-algo.local.bwr", localWindowBwr,
                                            "-algo.local.fwr", localWindowFwr
                                    };
    QStringList reprocProfileSplitterArgs = { "ReprocessedProfileSplitter",
                                              "-in", reprocTimeSeriesFileName,
                                              "-outlist", reprocFileListFileName,
                                              "-compress", "1"
                                    };
    QStringList fittedProfileReprocArgs = { "ProfileReprocessing",
                                            "-lai", allLaiTimeSeriesFileName,
                                            "-err", allErrTimeSeriesFileName,
                                            "-ilxml", listProducts.join(" "),
                                            "-opf", fittedTimeSeriesFileName,
                                            "-algo", "fit"
                                    };
    QStringList fittedProfileReprocSplitterArgs = { "ReprocessedProfileSplitter",
                                                    "-in", fittedTimeSeriesFileName,
                                                    "-outlist", fittedFileListFileName,
                                                    "-compress", "1"
                                    };

    const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");

    WriteExecutionInfosFile(executionInfosPath, parameters, configParameters, listProducts);
    QStringList productFormatterArgs = { "ProductFormatter",
                                    "-destroot", targetFolder,
                                    "-fileclass", "SVT1",
                                    "-level", "L3B",
                                    "-timeperiod", l3aSynthesisDate,
                                    "-baseline", "01.00",
                                    "-processor", "vegetation",
                                    "-processor.vegetation.lairepr", reprocTimeSeriesFileName,
                                    "-processor.vegetation.laifit", fittedTimeSeriesFileName,
                                    "-il", listProducts.join(" "),
                                    "-gipp", executionInfosPath
                                };
    // add these steps to the steps list to be submitted
    steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
    steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
    steps.append(profileReprocTask.CreateStep("ProfileReprocessing", profileReprocessingArgs));
    steps.append(profileReprocSplitTask.CreateStep("ReprocessedProfileSplitter", reprocProfileSplitterArgs));
    steps.append(fittedProfileReprocTask.CreateStep("ProfileReprocessing", fittedProfileReprocArgs));
    steps.append(fittedProfileReprocSplitTask.CreateStep("ReprocessedProfileSplitter", fittedProfileReprocSplitterArgs));
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);
}

void LaiRetrievalHandler::WriteExecutionInfosFile(const QString &executionInfosPath, const QJsonObject &parameters,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
        // Get the parameters from the configuration
        const auto &bwr = configParameters["processor.l3b.bwr"];
        const auto &fwr = configParameters["processor.l3a.fwr"];

        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <ProfileReprocessing_parameters>" << std::endl;
        executionInfosFile << "    <bwr_for_algo_local_online_retrieval>" << bwr.toStdString() << "</bwr_for_algo_local_online_retrieval>" << std::endl;
        executionInfosFile << "    <fwr_for_algo_local_online_retrieval>"<< fwr.toStdString() <<"</fwr_for_algo_local_online_retrieval>" << std::endl;
        executionInfosFile << "  </ProfileReprocessing_parameters>" << std::endl;

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

void LaiRetrievalHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
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

void LaiRetrievalHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
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

void LaiRetrievalHandler::GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters)
{
    // Get the models folder name
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    QStringList nameFilters = {strPattern};
    QDirIterator it(modelsFolder, nameFilters, QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        if (QFileInfo(it.filePath()).isFile()) {
            outListModels.append(it.filePath());
        }
    }
}


