#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "reflector_interface.h"

class Serialization : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<OrgEsaSen2agriReflectorInterface> client;

    static QString reflectorName;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void configurationParameterInfo();
    void configurationParameterInfoList();
    void configurationParameterValue();
    void configurationParameterValueList();
    void configurationCategory();
    void configurationCategoryList();
    void site();
    void siteList();
    void configurationSet();
    void configurationUpdateAction();
    void configurationUpdateActionList();
    void keyedMessage();
    void keyedMessageList();
    void product();
    void productList();
    void productToArchive();
    void productToArchiveList();
    void archivedProduct();
    void archivedProductList();
    void newJob();
    void newTask();
    void newStep();
    void newStepList();
    void executionStatusList();
    void executionStatistics();
    void taskAddedEvent();
    void taskFinishedEvent();
    void productAvailableEvent();
    void jobCancelledEvent();
    void jobPausedEvent();
    void jobResumedEvent();
    void jobSubmittedEvent();
    void stepFailedEvent();
    void unprocessedEvent();
    void unprocessedEventList();
    void nodeStatistics();
    void nodeStatisticsList();
    void newExecutorStep();
    void newExecutorStepList();
    void jobStepToRun();
    void jobStepToRunList();
};
