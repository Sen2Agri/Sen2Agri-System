#include <utility>

#include <QFileInfo>

#include <QLineEdit>
#include <QDateEdit>
#include <QCheckBox>

#include "parameterchangelistener.hpp"

using std::move;

ParameterChangeListener::ParameterChangeListener(ConfigModel &model,
                                                 ConfigurationParameterInfo parameter,
                                                 QWidget *widget,
                                                 QObject *parent)
    : QObject(parent), model(model), parameter(move(parameter)), widget(widget)
{
    isValid = true;

    if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
        connect(lineEdit, &QLineEdit::textChanged, this, &ParameterChangeListener::onTextChanged);
    } else if (auto dateEdit = qobject_cast<QDateEdit *>(widget)) {
        connect(dateEdit, &QDateEdit::dateChanged, this, &ParameterChangeListener::onDateChanged);
    } else if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
        connect(checkBox, &QCheckBox::stateChanged, this, &ParameterChangeListener::onBoolChanged);
    }
}

bool ParameterChangeListener::valid() const
{
    return isValid;
}

static bool validateInt(const QString &s);
static bool validateFloat(const QString &s);
static bool validatePath(const QString &s);

void ParameterChangeListener::onTextChanged(const QString &newText)
{
    bool valid;
    if (parameter.dataType == "int") {
        valid = validateInt(newText);
    } else if (parameter.dataType == "float") {
        valid = validateFloat(newText);
    } else if (parameter.dataType == "path") {
        valid = validatePath(newText);
    } else {
        valid = true;
    }

    if (valid) {
        model.setValue(parameter.key, newText);
    }

    applyValidity(valid);
}

void ParameterChangeListener::onDateChanged(const QDate &newDate)
{
    model.setValue(parameter.key, newDate.toString(Qt::ISODate));
}

void ParameterChangeListener::onBoolChanged(int newState)
{
    if (newState == Qt::Checked) {
        model.setValue(parameter.key, "true");
    } else {
        model.setValue(parameter.key, "false");
    }
}

void ParameterChangeListener::applyValidity(bool valid)
{
    if (valid != isValid) {
        isValid = valid;

        auto palette = widget->palette();
        if (valid) {
            palette.setColor(QPalette::Base, originalBaseColor);
        } else {
            originalBaseColor = palette.color(QPalette::Base);
            palette.setColor(QPalette::Base, Qt::red);
        }
        widget->setPalette(palette);
    }
}

static bool validateInt(const QString &s)
{
    bool ok;
    s.toInt(&ok);
    return ok;
}

static bool validateFloat(const QString &s)
{
    bool ok;
    s.toFloat(&ok);
    return ok;
}

static bool validatePath(const QString &s)
{
    return QFileInfo::exists(s);
}
