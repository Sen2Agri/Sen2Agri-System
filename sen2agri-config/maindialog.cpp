#include <iterator>

#include <QMessageBox>
#include <QProcess>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QTableWidget>
#include <QFormLayout>

#include "maindialog.hpp"
#include "ui_maindialog.h"
#include "parameterchangelistener.hpp"

using std::end;

static ConfigurationSet getStubConfiguration()
{
    ConfigurationSet configuration;

    configuration.categories.append({ 1, "General" });
    configuration.categories.append({ 2, "Not used" });
    configuration.categories.append({ 3, "L2A" });

    configuration.parameters.append({ "test.foo", 1, "Foo", "string", "val 1", false });
    configuration.parameters.append({ "test.bar", 1, "Boo", "string", "val 2", false });
    configuration.parameters.append({ "test.qux", 1, "Qux", "int", "42", false });
    configuration.parameters.append({ "test.date", 1, "Date", "date", "2014-02-03", false });
    configuration.parameters.append({ "test.date.ro", 1, "Date RO", "date", "2014-02-03", true });
    configuration.parameters.append({ "test.bool.rw", 1, "Bool", "bool", "true", false });
    configuration.parameters.append({ "test.bool.ro", 1, "Bool", "bool", "true", true });

    configuration.parameters.append({ "test.baz", 3, "Baz", "string", "val 2", true });

    return configuration;
}

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainDialog),
      clientInterface(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                      QStringLiteral("/"),
                      QDBusConnection::systemBus())
{
    ui->setupUi(this);

    auto promise = clientInterface.GetConfigurationSet();
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise]() {
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

    QMap<int, QFormLayout *> tabs;
    for (const auto &cat : configModel.categories()) {
        if (usedCategories.contains(cat.categoryId)) {
            auto widget = new QWidget(ui->tabWidget);
            auto layout = new QFormLayout(widget);
            ui->tabWidget->addTab(widget, cat.name);
            tabs.insert(cat.categoryId, layout);
        }
    }

    auto endTabs = end(tabs);
    for (const auto &param : configModel.parameters()) {
        auto it = tabs.find(param.categoryId);
        if (it != endTabs) {
            auto layout = *it;

            auto widget = layout->parentWidget();
            if (auto editWidget = createWidgetForParameter(param, widget)) {
                layout->addRow(new QLabel(param.friendlyName, widget), editWidget);
            } else {
                QMessageBox::warning(
                    this, QStringLiteral("Error"),
                    QStringLiteral(
                        "Unable to create editor widget for parameter %1 with data type %2")
                        .arg(param.key)
                        .arg(param.dataType));
            }
        }
    }
}

void MainDialog::done(int result)
{
    if (result == QDialog::Accepted) {
        auto isValid = true;
        for (const auto *l : parameterChangeListeners) {
            if (!l->valid()) {
                isValid = false;
                break;
            }
        }

        if (isValid) {
            saveChanges();
        } else {
            QMessageBox::warning(this, QStringLiteral("Error"),
                                 QStringLiteral("Please make sure that the parameters are valid"));
        }
    } else {
        if (configModel.getNewValues().size() == 0 ||
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
    auto promise = clientInterface.UpdateConfigurationParameters(configModel.getChanges());
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise]() {
                if (promise.isValid()) {
                    QDialog::done(QDialog::Accepted);
                } else if (promise.isError()) {
                    QMessageBox::warning(this, QStringLiteral("Error"),
                                         QStringLiteral("Unable to save the changes: %1")
                                             .arg(promise.error().message()));
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

static void makeWidgetReadOnly(QLineEdit *widget);
static void makeWidgetReadOnly(QDateEdit *widget);
static void makeWidgetReadOnly(QCheckBox *widget);

QWidget *MainDialog::createWidgetForParameter(const ConfigurationParameterInfo &parameter,
                                              QWidget *parent)
{
    if (parameter.dataType == "int" || parameter.dataType == "float" ||
        parameter.dataType == "string" || parameter.dataType == "path") {
        auto widget = new QLineEdit(parameter.value, parent);
        if (parameter.isAdvanced) {
            makeWidgetReadOnly(widget);
        } else {
            parameterChangeListeners.append(
                new ParameterChangeListener(configModel, parameter, widget, widget));
        }
        return widget;
    } else if (parameter.dataType == "date") {
        auto widget = new QDateEdit(QDate::fromString(parameter.value, Qt::ISODate), parent);
        if (parameter.isAdvanced) {
            makeWidgetReadOnly(widget);
        } else {
            new ParameterChangeListener(configModel, parameter, widget, widget);
        }
        return widget;
    } else if (parameter.dataType == "bool") {
        auto widget = new QCheckBox();
        const auto &lc = parameter.value.toLower();
        if (lc == "t" || lc == "true" || lc == "y" || lc == "yes" || lc == "on" || lc == "1") {
            widget->setCheckState(Qt::Checked);
            if (parameter.isAdvanced) {
                makeWidgetReadOnly(widget);
            } else {
                new ParameterChangeListener(configModel, parameter, widget, widget);
            }
        }
        return widget;
    }

    return nullptr;
}

static QColor getReadOnlyBackgroundColor()
{
    return QColor(230, 230, 230);
}

static void setReadOnlyPalette(QWidget *widget)
{
    auto palette = widget->palette();
    palette.setColor(QPalette::Base, getReadOnlyBackgroundColor());
    widget->setPalette(palette);
}

static void makeWidgetReadOnly(QLineEdit *widget)
{
    //    widget->setDisabled(true);
    widget->setReadOnly(true);
    setReadOnlyPalette(widget);
}

static void makeWidgetReadOnly(QDateEdit *widget)
{
    widget->setReadOnly(true);
    setReadOnlyPalette(widget);
}

static void makeWidgetReadOnly(QCheckBox *widget)
{
    widget->setEnabled(false);
    setReadOnlyPalette(widget);
}
