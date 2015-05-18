#pragma once

#include <QDialog>
#include <QList>

#include "configmodel.hpp"
#include "parameterchangelistener.hpp"
#include "persistencemanager_interface.h"

namespace Ui
{
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

private slots:
    void done(int result);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::MainDialog *ui;
    ConfigModel configModel;
    QList<ParameterChangeListener *> parameterChangeListeners;
    OrgEsaSen2agriPersistenceManagerInterface clientInterface;

    void loadModel(const ConfigurationSet &configuration);
    QWidget *createWidgetForParameter(const ConfigurationParameterInfo &parameter, QWidget *parent);
    void saveChanges();
};
