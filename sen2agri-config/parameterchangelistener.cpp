#include <utility>

#include <QFileInfo>

#include <QLineEdit>
#include <QDateEdit>
#include <QCheckBox>

#include "parameterchangelistener.hpp"

static bool validateInt(const QString &s);
static bool validateFloat(const QString &s);
static bool validatePath(const QString &s);
static bool validateTrue(const QString &s);

ParameterChangeListener::ParameterChangeListener(ConfigModel &model,
                                                 ConfigurationParameterInfo parameter,
                                                 ParameterKey parameterKey,
                                                 QString friendlyName,
                                                 QWidget *parent)
    : QObject(parent),
      model(model),
      parameterKey(std::move(parameterKey)),
      friendlyName(std::move(friendlyName)),
      widget(parent)
{
    isValid = true;

    if (parameter.dataType == "int") {
        validate = validateInt;
    } else if (parameter.dataType == "float") {
        validate = validateFloat;
    } else if (parameter.dataType == "path") {
        validate = validatePath;
    } else {
        validate = validateTrue;
    }

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

void ParameterChangeListener::onTextChanged(const QString &newText)
{
    auto valid = validate(newText);

    if (valid && widget->isEnabled()) {
        model.setValue(parameterKey, newText);
    }

    if (valid != isValid) {
        validityChanged(valid);
        applyValidity(valid);
    }
}

void ParameterChangeListener::onDateChanged(const QDate &newDate)
{
    model.setValue(parameterKey, newDate.toString(Qt::ISODate));
}

void ParameterChangeListener::onBoolChanged(int newState)
{
    if (newState == Qt::Checked) {
        model.setValue(parameterKey, "true");
    } else {
        model.setValue(parameterKey, "false");
    }
}

void ParameterChangeListener::applyValidity(bool valid)
{
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

const QString &ParameterChangeListener::parameterName() const
{
    return friendlyName;
}

const ParameterKey &ParameterChangeListener::key() const
{
    return parameterKey;
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

static bool validateTrue(const QString &)
{
    return true;
}
