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

#include "maindialog.hpp"
#include "ui_maindialog.h"
#include "parameterchangelistener.hpp"

using std::end;

static void addParameter(ConfigurationSet &configuration,
                         const ConfigurationParameterInfo &parameter,
                         std::experimental::optional<int> siteId,
                         const QString &value)
{
    auto found = false;
    for (const auto &p : configuration.parameterInfo) {
        if (p.key == parameter.key) {
            found = true;
            break;
        }
    }

    if (!found) {
        configuration.parameterInfo.append(parameter);
    }

    configuration.parameterValues.append({ parameter.key, siteId, value });
}

static ConfigurationSet getStubConfiguration()
{
    ConfigurationSet configuration;

    configuration.categories.append({ 1, "General" });
    configuration.categories.append({ 2, "Not used" });
    configuration.categories.append({ 3, "L2A" });

    addParameter(configuration, { "test.foo", 1, "Foo", "string", false },
                 std::experimental::nullopt, "Foo value");
    addParameter(configuration, { "test.foo", 1, "Foo", "string", false }, 1,
                 "Foo value for site 1");
    addParameter(configuration, { "test.bar", 1, "Bar", "file", false }, std::experimental::nullopt,
                 "/etc/sen2agri/sen2agri-persistence.conf");
    addParameter(configuration, { "test.baz", 1, "Baz", "directory", false },
                 std::experimental::nullopt, "/etc/sen2agri");
    addParameter(configuration, { "test.qux", 1, "Qux", "int", false }, std::experimental::nullopt,
                 "12");
    addParameter(configuration, { "test.date", 1, "Date", "date", false },
                 std::experimental::nullopt, "2015-01-02");
    addParameter(configuration, { "test.date.ro", 1, "Date RO", "date", true },
                 std::experimental::nullopt, "2015-05-05");
    addParameter(configuration, { "test.bool", 1, "Bool", "bool", false },
                 std::experimental::nullopt, "false");
    addParameter(configuration, { "test.bool.ro", 1, "Bool RO", "bool", true },
                 std::experimental::nullopt, "true");
    addParameter(configuration, { "test.quux", 3, "Quux", "string", true },
                 std::experimental::nullopt, "hello");

    configuration.sites.append({ 1, "Site 1" });
    configuration.sites.append({ 2, "Site 2" });

    return configuration;
}

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainDialog),
      clientInterface(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                      QStringLiteral("/"),
                      QDBusConnection::systemBus()),
      invalidFields(),
      isAdmin()
{
    ui->setupUi(this);

    auto promise = clientInterface.GetConfigurationSet();
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise] {
                if (promise.isValid()) {
                    loadModel(promise.value());
                } else if (promise.isError()) {
#if 1

                    loadModel(getStubConfiguration());
#else
                    ui->innerLayout->removeWidget(ui->tabWidget);

                    auto spacer = new QWidget(this);
                    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
                    ui->innerLayout->addWidget(spacer, 0, 0);

                    ui->innerLayout->addWidget(
                        new QLabel(
                            QStringLiteral("An error occurred while loading the configuration:"),
                            this),
                        1, 0, Qt::AlignHCenter);

                    ui->innerLayout->addWidget(new QLabel(promise.error().message(), this), 2, 0,
                                               Qt::AlignHCenter);

                    spacer = new QWidget(this);
                    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
                    ui->innerLayout->addWidget(spacer, 3, 0);

                    ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
#endif
                }
            });
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::loadModel(const ConfigurationSet &configuration)
{
    configModel = { configuration };

    QSet<int> usedCategories;
    for (const auto &param : configModel.parameters()) {
        usedCategories.insert(param.categoryId);
    }

    for (const auto &cat : configModel.categories()) {
        if (usedCategories.contains(cat.categoryId)) {
            auto widget = new QWidget(ui->tabWidget);
            auto parentLayout = new QFormLayout(widget);
            auto regionList = new QComboBox(widget);
            regionList->addItem(QString("[Global]"), 0);
            for (const auto &site : configModel.sites()) {
                regionList->addItem(site.name, site.siteId);
            }
            connect(regionList,
                    static_cast<void (QComboBox::*) (int) >(&QComboBox::currentIndexChanged),
                    [this, regionList, widget](int) {
                        parameterChangeListeners.clear();

                        std::experimental::optional<int> siteId;
                        if (auto siteIdVal = regionList->currentData().toInt()) {
                            siteId = siteIdVal;
                        }

                        auto categoryId = tabCategory[ui->tabWidget->currentIndex()];
                        switchSite(siteId, categoryId, widget);
                    });
            regionLists.emplace_back(regionList);

            parentLayout->addRow(new QLabel(QStringLiteral("Site"), widget), regionList);
            parentLayout->addRow(
                createFieldsWidget(std::experimental::nullopt, cat.categoryId, widget));

            ui->tabWidget->addTab(widget, cat.name);
            tabCategory.emplace_back(cat.categoryId);
        }
    }
}

void MainDialog::switchSite(std::experimental::optional<int> siteId,
                            int categoryId,
                            QWidget *parentWidget)
{
    auto layout = static_cast<QFormLayout *>(parentWidget->layout());
    auto item = layout->itemAt(1, QFormLayout::SpanningRole);
    layout->removeItem(item);
    item->widget()->deleteLater();
    delete item;
    layout->setWidget(1, QFormLayout::SpanningRole,
                      createFieldsWidget(siteId, categoryId, parentWidget));
}

QWidget *MainDialog::createFieldsWidget(std::experimental::optional<int> siteId,
                                        int categoryId,
                                        QWidget *parentWidget)
{
    auto fieldsWidget = new QWidget(parentWidget);
    auto layout = new QFormLayout(fieldsWidget);

    for (const auto &param : configModel.parameters()) {
        if (param.categoryId == categoryId) {
            if (auto editWidget = createEditRow(param, { param.key, siteId }, fieldsWidget)) {
                layout->addRow(new QLabel(param.friendlyName, fieldsWidget), editWidget);
            } else {
                QMessageBox::critical(
                    this, QStringLiteral("Error"),
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
        for (const auto *l : parameterChangeListeners) {
            if (!l->valid()) {
                errors += QStringLiteral("%1\n").arg(l->parameterName());
            }
        }

        if (errors.isEmpty()) {
            saveChanges();
        } else {
            QMessageBox::critical(
                this, QStringLiteral("Error"),
                QStringLiteral("Please make sure that the following parameters are valid:\n\n") +
                    errors);
        }
    } else {
        if (!configModel.hasChanges() ||
            QMessageBox::question(this, QStringLiteral("Save changes"),
                                  QStringLiteral("Are you sure you want to close the "
                                                 "application without saving the changes?"),
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            QDialog::done(result);
        }
    }
}

void MainDialog::saveChanges()
{
    setEnabled(false);
    auto promise = clientInterface.UpdateConfigurationParameters(configModel.getChanges());
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise] {
                setEnabled(true);

                if (promise.isError()) {
                    QMessageBox::critical(this, QStringLiteral("Error"),
                                          QStringLiteral("Unable to save the changes: %1")
                                              .arg(promise.error().message()));
                } else if (promise.isValid()) {
                    const auto &result = promise.value();
                    if (result.empty()) {
                        configModel.reset();
                        QMessageBox::information(
                            this, QStringLiteral("Information"),
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

QWidget *MainDialog::createEditRow(const ConfigurationParameterInfo &parameter,
                                   const ParameterKey &parameterKey,
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
        auto listener = new ParameterChangeListener(configModel, parameter, parameterKey,
                                                    parameter.friendlyName, editWidget);
        connect(listener, &ParameterChangeListener::validityChanged, [this](bool isValid) {
            auto idx = ui->tabWidget->currentIndex();
            if (isValid) {
                if (!--invalidFields) {
                    regionLists[idx]->setEnabled(true);
                }
            } else {
                if (!invalidFields++) {
                    regionLists[idx]->setEnabled(false);
                }
            }
        });
        parameterChangeListeners.append(listener);
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

void MainDialog::toggleSiteSpecific(const ParameterKey &parameter,
                                    QPushButton *button,
                                    QWidget *widget)
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
