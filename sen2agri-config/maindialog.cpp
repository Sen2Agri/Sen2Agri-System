#include <iterator>

#include <QMessageBox>
#include <QProcess>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QFormLayout>

#include "maindialog.hpp"
#include "ui_maindialog.h"
#include "persistencemanager_interface.h"

using std::end;

MainDialog::MainDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    auto interface = new OrgEsaSen2agriPersistenceManagerInterface(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(), QStringLiteral("/"),
        QDBusConnection::systemBus());

    auto promise = interface->GetConfigurationSet();
    connect(
        new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
        [this, promise]() {
            if (promise.isValid()) {
                configModel = { promise.value() };

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
                        layout->addRow(new QLabel(param.friendlyName, widget),
                                       new QLineEdit(param.value, widget));
                    }
                }
            } else if (promise.isError()) {
                ui->innerLayout->removeWidget(ui->tabWidget);

                auto spacer = new QWidget(this);
                spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
                ui->innerLayout->addWidget(spacer, 0, 0);

                ui->innerLayout->addWidget(
                    new QLabel(QStringLiteral("An error occurred while loading the configuration:"),
                               this),
                    1, 0, Qt::AlignHCenter);

                ui->innerLayout->addWidget(new QLabel(promise.error().message(), this), 2, 0,
                                           Qt::AlignHCenter);

                spacer = new QWidget(this);
                spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
                ui->innerLayout->addWidget(spacer, 3, 0);

                ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
            }
        });
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::on_buttonBox_accepted()
{
    this->setEnabled(false);
}

void MainDialog::on_buttonBox_rejected()
{
    reject();
}
