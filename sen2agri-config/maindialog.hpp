#pragma once

#include <QDialog>
#include <QList>
#include <QComboBox>

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
    std::vector<int> tabCategory;
    std::vector<QComboBox *> regionLists;
    int invalidFields;
    bool isAdmin;

    void loadModel(const ConfigurationSet &configuration);
    void switchSite(std::experimental::optional<int> siteId, int categoryId, QWidget *parentWidget);

    void toggleSiteSpecific(const ParameterKey &parameter, QPushButton *button, QWidget *widget);

    void displayAsGlobal(QPushButton *button, QWidget *widget);
    void displayAsSiteSpecific(QPushButton *button, QWidget *widget);

    void applyValue(QWidget *editWidget, const QString &value);

    QWidget *createFieldsWidget(std::experimental::optional<int> siteId,
                                int categoryId,
                                QWidget *parentWidget);
    QWidget *createEditRow(const ConfigurationParameterInfo &parameter,
                           const ParameterKey &parameterKey,
                           QWidget *parent);
    void saveChanges();
};
