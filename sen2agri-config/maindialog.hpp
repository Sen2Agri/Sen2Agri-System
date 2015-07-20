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
    OrgEsaSen2agriPersistenceManagerInterface clientInterface;
    std::vector<int> tabCategory;
    std::map<int, std::vector<ParameterChangeListener *>> parameterChangeListeners;
    std::vector<QComboBox *> siteLists;
    int invalidFields;
    bool isAdmin;

    void loadConfiguration(int currentTab, int currentSite);
    void loadModel(const ConfigurationSet &configuration);
    void switchSite(std::experimental::optional<int> siteId,
                    int categoryId,
                    QComboBox *siteList,
                    QWidget *parentWidget);

    void toggleSiteSpecific(const ParameterKey &parameter, QPushButton *button, QWidget *widget);

    void displayAsGlobal(QPushButton *button, QWidget *widget);
    void displayAsSiteSpecific(QPushButton *button, QWidget *widget);

    void applyValue(QWidget *editWidget, const QString &value);

    QComboBox *createSiteList(int categoryId, QWidget *parent);
    QWidget *createFieldsWidget(std::experimental::optional<int> siteId,
                                int categoryId,
                                QComboBox *siteList,
                                QWidget *parentWidget);
    QWidget *createEditRow(int categoryId,
                           const ConfigurationParameterInfo &parameter,
                           const ParameterKey &parameterKey,
                           QComboBox *siteList,
                           QWidget *parent);
    void saveChanges();
};
