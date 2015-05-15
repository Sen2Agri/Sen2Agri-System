#pragma once

#include <QDialog>

#include "configmodel.hpp"

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
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::MainDialog *ui;
    ConfigModel configModel;
};
