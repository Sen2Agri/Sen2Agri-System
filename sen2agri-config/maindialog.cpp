#include <iterator>

#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QTableWidget>
#include <QFormLayout>
#include <QtConcurrent>
#include <QFutureWatcher>

#include <optional.hpp>

#include "maindialog.hpp"
#include "ui_maindialog.h"
#include "parameterchangelistener.hpp"
#include "settings.hpp"
#include "configuration.hpp"
#include "runtimeerror.hpp"

using std::end;

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainDialog),
      clientInterface(Settings::readSettings(getConfigurationFile(*QCoreApplication::instance()))),
      invalidFields(),
      isAdmin()
{
    ui->setupUi(this);

    loadConfiguration(0, 0);
}

MainDialog::~MainDialog()
{
    delete ui;
}


void MainDialog::loadConfiguration(int currentTab, int currentSite)
{
    setEnabled(false);
    ui->tabWidget->clear();

    auto future = QtConcurrent::run([this]() {
        try
        {
            return clientInterface.GetConfigurationSet();
        }
        catch (const std::exception &e)
        {
            throw RuntimeError(e.what());
        }
    });

    auto watcher = new QFutureWatcher<ConfigurationSet>(this);
    watcher->setFuture(future);
    connect(watcher,
            &QFutureWatcher<ConfigurationSet>::finished,
            [this, future, currentTab, currentSite] {
        setEnabled(true);

        try
        {
            loadModel(future.result());

            if (currentTab < ui->tabWidget->count()) {
                ui->tabWidget->setCurrentIndex(currentTab);

                if (currentSite) {
                    auto siteList = siteLists[currentTab];
                    auto siteCount = siteList->count();
                    for (int i = 0; i < siteCount; i++) {
                        if (siteList->itemData(i).toInt() == currentSite) {
                            siteList->setCurrentIndex(i);
                            break;
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            ui->innerLayout->removeWidget(ui->tabWidget);

            auto spacer = new QWidget(this);
            spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            ui->innerLayout->addWidget(spacer, 0, 0);

            ui->innerLayout->addWidget(
                new QLabel(QStringLiteral("An error occurred while loading the configuration:"),
                           this),
                1,
                0,
                Qt::AlignHCenter);

            ui->innerLayout->addWidget(new QLabel(e.what(), this), 2, 0, Qt::AlignHCenter);

            spacer = new QWidget(this);
            spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            ui->innerLayout->addWidget(spacer, 3, 0);

            ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        }
    });
}

void MainDialog::loadModel(const ConfigurationSet &configuration)
{
    tabCategory.clear();
    parameterChangeListeners.clear();
    siteLists.clear();

    configModel = { configuration };

    QSet<int> usedCategories;
    for (const auto &param : configModel.parameters()) {
        usedCategories.insert(param.categoryId);
    }

    for (const auto &cat : configModel.categories()) {
        if (usedCategories.contains(cat.categoryId)) {
            auto widget = new QWidget(ui->tabWidget);
            auto parentLayout = new QFormLayout(widget);

            if (cat.allowPerSiteCustomization) {
                auto siteList = createSiteList(cat.categoryId, widget);
                parentLayout->addRow(new QLabel(QStringLiteral("Site"), widget), siteList);
                siteLists.emplace_back(siteList);
            } else {
                siteLists.emplace_back(nullptr);
            }

            parentLayout->addRow(createFieldsWidget(
                std::experimental::nullopt, cat.categoryId, siteLists.back(), widget));

            ui->tabWidget->addTab(widget, cat.name);
            tabCategory.emplace_back(cat.categoryId);
        }
    }
}

QComboBox *MainDialog::createSiteList(int categoryId, QWidget *parent)
{
    auto siteList = new QComboBox(parent);
    siteList->addItem(QStringLiteral("[Global]"), 0);
    for (const auto &site : configModel.sites()) {
        siteList->addItem(site.name, site.siteId);
    }

    parameterChangeListeners.emplace(
        std::make_pair(categoryId, std::vector<ParameterChangeListener *>()));

    connect(siteList,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this, siteList](int) {
        std::experimental::optional<int> siteId;
        if (auto siteIdVal = siteList->currentData().toInt()) {
            siteId = siteIdVal;
        }

        auto categoryId = tabCategory[ui->tabWidget->currentIndex()];
        for (const auto l : parameterChangeListeners[categoryId]) {
            delete l;
        }
        parameterChangeListeners[categoryId].clear();

        auto widget = ui->tabWidget->currentWidget();
        switchSite(siteId, categoryId, siteList, widget);
    });

    return siteList;
}

void MainDialog::switchSite(std::experimental::optional<int> siteId,
                            int categoryId,
                            QComboBox *siteList,
                            QWidget *parentWidget)
{
    auto layout = static_cast<QFormLayout *>(parentWidget->layout());
    auto item = layout->itemAt(1, QFormLayout::SpanningRole);
    layout->removeItem(item);
    item->widget()->deleteLater();
    delete item;
    layout->setWidget(1,
                      QFormLayout::SpanningRole,
                      createFieldsWidget(siteId, categoryId, siteList, parentWidget));
}

QWidget *MainDialog::createFieldsWidget(std::experimental::optional<int> siteId,
                                        int categoryId,
                                        QComboBox *siteList,
                                        QWidget *parentWidget)
{
    auto fieldsWidget = new QWidget(parentWidget);
    auto layout = new QFormLayout(fieldsWidget);

    for (const auto &param : configModel.parameters()) {
        if (param.categoryId == categoryId) {
            if (auto editWidget = createEditRow(
                    categoryId, param, { param.key, siteId }, siteList, fieldsWidget)) {
                layout->addRow(new QLabel(param.friendlyName, fieldsWidget), editWidget);
            } else {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    QStringLiteral(
                        "Unable to create editor widget for parameter %1 with data type %2")
                        .arg(param.key)
                        .arg(param.dataType));
            }
        }
    }

    return fieldsWidget;
}

void MainDialog::done(int result)
{
    if (result == QDialog::Accepted) {
        QString errors;

        std::experimental::optional<int> errorTab;

        auto currentTab = ui->tabWidget->currentIndex();
        auto currentTabHasErrors = false;

        auto tabCount = ui->tabWidget->count();
        for (int i = 0; i < tabCount; i++) {
            auto categoryId = tabCategory[i];
            for (const auto *l : parameterChangeListeners[categoryId]) {
                if (!l->valid()) {
                    errors += QStringLiteral("%1\n").arg(l->parameterName());

                    if (!errorTab) {
                        errorTab = i;
                    }

                    if (i == currentTab) {
                        currentTabHasErrors = true;
                    }
                }
            }
        }

        if (errors.isEmpty()) {
            saveChanges();
        } else {
            if (errorTab && !currentTabHasErrors) {
                ui->tabWidget->setCurrentIndex(*errorTab);
            }

            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                QStringLiteral("Please make sure that the following parameters are valid:\n\n") +
                    errors);
        }
    } else {
        if (!configModel.hasChanges() ||
            QMessageBox::question(this,
                                  QStringLiteral("Save changes"),
                                  QStringLiteral("Are you sure you want to close the "
                                                 "application without saving the changes?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::Yes) {
            QDialog::done(result);
        }
    }
}

void MainDialog::saveChanges()
{
    setEnabled(false);

    auto currentTab = ui->tabWidget->currentIndex();
    auto siteList = siteLists[currentTab];
    int currentSite = 0;
    if (siteList) {
        currentSite = siteList->currentData().toInt();
    }

    auto future = QtConcurrent::run([this]() {
        try
        {
            return clientInterface.UpdateConfigurationParameters(configModel.getChanges(), true);
        }
        catch (const std::exception &e)
        {
            throw RuntimeError(e.what());
        }
    });

    auto watcher = new QFutureWatcher<KeyedMessageList>(this);
    watcher->setFuture(future);
    connect(watcher,
            &QFutureWatcher<KeyedMessageList>::finished,
            [this, future, currentTab, currentSite] {
        setEnabled(true);

        try
        {
            const auto &result = future.result();
            if (result.empty()) {
                loadConfiguration(currentTab, currentSite);

                QMessageBox::information(this,
                                         QStringLiteral("Information"),
                                         QStringLiteral("The changes were saved successfully"));
            } else {
                auto message = QStringLiteral("The following %1 could not be saved:\n\n")
                                   .arg(result.size() == 1 ? QStringLiteral("parameter")
                                                           : QStringLiteral("parameters"));

                for (const auto &e : result) {
                    message += QStringLiteral("%1: %2\n").arg(e.key, e.text);
                }

                QMessageBox::critical(this, QStringLiteral("Error"), message);
            }
        }
        catch (const std::exception &e)
        {
            QMessageBox::critical(this,
                                  QStringLiteral("Error"),
                                  QStringLiteral("Unable to save the changes: %1").arg(e.what()));
        }
    });
}

void MainDialog::on_buttonBox_accepted()
{
    accept();
}

void MainDialog::on_buttonBox_rejected()
{
    reject();
}

QWidget *MainDialog::createEditRow(int categoryId,
                                   const ConfigurationParameterInfo &parameter,
                                   const ParameterKey &parameterKey,
                                   QComboBox *siteList,
                                   QWidget *parent)
{
    auto isDisabled = parameter.isAdvanced && !configModel.isAdmin();

    auto container = new QWidget(parent);
    auto layout = new QHBoxLayout(container);
    QWidget *editWidget;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStrut(QLineEdit().sizeHint().height());

    if (parameter.dataType == QLatin1String("int") ||
        parameter.dataType == QLatin1String("float") ||
        parameter.dataType == QLatin1String("string")) {
        auto widget = new QLineEdit(container);
        editWidget = widget;
        layout->addWidget(widget);
        if (isDisabled) {
            widget->setEnabled(false);
        }
    } else if (parameter.dataType == QLatin1String("file") ||
               parameter.dataType == QLatin1String("directory")) {
        if (isDisabled) {
            auto widget = new QLineEdit(container);
            editWidget = widget;
            layout->addWidget(widget);
            widget->setEnabled(false);
        } else {
            auto widget = new QLineEdit(container);
            editWidget = widget;
            layout->addWidget(widget);
            auto button = new QPushButton(QStringLiteral("..."), container);
            layout->addWidget(button);
            if (parameter.dataType == QLatin1String("file")) {
                connect(button, &QPushButton::clicked, [this, widget] {
                    const auto &file = QFileDialog::getOpenFileName(this);
                    if (!file.isNull()) {
                        widget->setText(file);
                    }
                });
            } else {
                connect(button, &QPushButton::clicked, [this, widget] {
                    const auto &directory = QFileDialog::getExistingDirectory(this);
                    if (!directory.isNull()) {
                        widget->setText(directory);
                    }
                });
            }
        }
    } else if (parameter.dataType == QLatin1String("date")) {
        auto widget = new QDateEdit(container);
        editWidget = widget;
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(widget);
        if (isDisabled) {
            widget->setEnabled(false);
        }
    } else if (parameter.dataType == QLatin1String("bool")) {
        auto widget = new QCheckBox(container);
        editWidget = widget;
        layout->addWidget(widget);
        if (isDisabled) {
            widget->setEnabled(false);
        }
    } else {
        container->deleteLater();
        return nullptr;
    }

    if (!isDisabled) {
        auto listener = new ParameterChangeListener(
            configModel, parameter, parameterKey, parameter.friendlyName, editWidget);
        if (siteList) {
            connect(listener,
                    &ParameterChangeListener::validityChanged,
                    [this, siteList](bool isValid) {
                if (isValid) {
                    if (!--invalidFields) {
                        siteList->setEnabled(true);
                    }
                } else {
                    invalidFields++;
                    siteList->setEnabled(false);
                }
            });
        }
        parameterChangeListeners[categoryId].emplace_back(listener);
    }

    bool fromGlobal;
    const auto &value = configModel.getValue(parameterKey, fromGlobal);

    if (parameterKey.siteId() != std::experimental::nullopt && !isDisabled) {
        auto button = new QPushButton(container);

        auto customizeString = QStringLiteral("Customize");
        const auto &textSize = button->fontMetrics().size(Qt::TextShowMnemonic, customizeString);
        QStyleOptionButton opt;
        opt.initFrom(button);
        opt.rect.setSize(textSize);
        button->setMinimumSize(
            button->style()->sizeFromContents(QStyle::CT_PushButton, &opt, textSize, button));

        if (fromGlobal) {
            displayAsGlobal(button, editWidget);
        } else {
            displayAsSiteSpecific(button, editWidget);
        }

        connect(button, &QPushButton::clicked, [this, parameterKey, button, editWidget] {
            toggleSiteSpecific(parameterKey, button, editWidget);
        });

        layout->addWidget(button, 0, Qt::AlignRight);
    }

    applyValue(editWidget, value);

    return container;
}

void
MainDialog::toggleSiteSpecific(const ParameterKey &parameter, QPushButton *button, QWidget *widget)
{
    const auto &globalValue = configModel.getGlobalValue(parameter);

    if (configModel.isSiteSpecific(parameter)) {
        configModel.removeValue(parameter);

        displayAsGlobal(button, widget);

        applyValue(widget, globalValue);
    } else {
        configModel.setValue(parameter, globalValue);

        displayAsSiteSpecific(button, widget);
    }
}

void MainDialog::displayAsGlobal(QPushButton *button, QWidget *widget)
{
    button->setText(QStringLiteral("Customize"));
    widget->setEnabled(false);
}

void MainDialog::displayAsSiteSpecific(QPushButton *button, QWidget *widget)
{
    button->setText(QStringLiteral("Reset"));
    widget->setEnabled(true);
}

void MainDialog::applyValue(QWidget *editWidget, const QString &value)
{
    if (auto lineEdit = qobject_cast<QLineEdit *>(editWidget)) {
        lineEdit->setText(value);
    } else if (auto dateEdit = qobject_cast<QDateEdit *>(editWidget)) {
        dateEdit->setDate(QDate::fromString(value, Qt::ISODate));
    } else if (auto checkBox = qobject_cast<QCheckBox *>(editWidget)) {
        const auto &lc = value.toLower();
        auto checked = lc == QLatin1String("t") || lc == QLatin1String("true") ||
                       lc == QLatin1String("y") || lc == QLatin1String("yes") ||
                       lc == QLatin1String("on") || lc == QLatin1String("1");
        checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}
