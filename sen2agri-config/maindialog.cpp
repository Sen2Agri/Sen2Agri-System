#include <iterator>

#include <QMessageBox>
#include <QProcess>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QFormLayout>

#include "maindialog.hpp"
#include "ui_maindialog.h"

using std::end;

MainDialog::MainDialog(ConfigModel &configModel, QWidget *parent)
    : QDialog(parent), ui(new Ui::MainDialog), configModel(configModel)
{
    ui->setupUi(this);

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

    auto endTabs = std::end(tabs);
    for (const auto p : configModel.parameters()) {
        auto it = tabs.find(p.categoryId);
        if (it != endTabs) {
            auto layout = *it;

            auto widget = layout->parentWidget();
            layout->addRow(new QLabel(p.friendlyName, widget), new QLineEdit(p.value, widget));
        }
    }
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
