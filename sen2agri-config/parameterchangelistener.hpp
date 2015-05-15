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
    ConfigurationParameterInfo parameter;
    QWidget *widget;
    QColor originalBaseColor;
    bool isValid;

public:
    explicit ParameterChangeListener(ConfigModel &model,
                                     ConfigurationParameterInfo parameterKey,
                                     QWidget *widget,
                                     QObject *parent = 0);

    bool valid() const;
    void applyValidity(bool valid);

public slots:
    void onTextChanged(const QString &newText);
    void onDateChanged(const QDate &newDate);
    void onBoolChanged(int newState);
};
