#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QDate>

#include "configmodel.hpp"

class ParameterChangeListener : public QObject
{
    Q_OBJECT

    ConfigModel &model;
    ParameterKey parameterKey;
    QString friendlyName;
    QWidget *widget;
    QColor originalBaseColor;
    bool isValid;
    bool (*validate)(const QString &value);

public:
    explicit ParameterChangeListener(ConfigModel &model,
                                     ConfigurationParameterInfo parameter,
                                     ParameterKey parameterKey,
                                     QString friendlyName,
                                     QWidget *parent);

    bool valid() const;
    void applyValidity(bool valid);

    const QString &parameterName() const;
    const ParameterKey &key() const;

signals:
    void validityChanged(bool isValid);

public slots:
    void onTextChanged(const QString &newText);
    void onDateChanged(const QDate &newDate);
    void onBoolChanged(int newState);
};
