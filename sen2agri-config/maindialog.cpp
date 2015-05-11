#include <QMessageBox>
#include <QProcess>

#include "maindialog.h"
#include "ui_maindialog.h"

MainDialog::MainDialog(ConfigModel &configModel, QWidget *parent)
    : QDialog(parent), ui(new Ui::MainDialog), configModel(configModel)
{
    ui->setupUi(this);

    ui->lineEdit->setText(configModel.value1);
    ui->lineEdit_2->setText(configModel.value2);
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::on_buttonBox_accepted()
{
    configModel.value1 = ui->lineEdit->text();
    configModel.value2 = ui->lineEdit_2->text();

    this->setEnabled(false);

    auto *p = new QProcess;
    connect(p, &QProcess::started, [=]() { p->write(configModel.serialize().toJson()); });
    connect(p, &QProcess::bytesWritten, [=]() { p->closeWriteChannel(); });
    connect(p, static_cast<void (QProcess::*) (int) >(&QProcess::finished), [=](int) {
        p->deleteLater();
        accept();
    });

    p->start("pkexec", QStringList() << QCoreApplication::instance()->applicationFilePath()
                                     << QStringLiteral("write"));
}

void MainDialog::on_buttonBox_rejected()
{
    reject();
}
