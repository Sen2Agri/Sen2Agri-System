QT       += core gui widgets dbus

TARGET = sen2agri-config
TEMPLATE = app

DESTDIR = bin

CONFIG += c++11 precompile_header

PRECOMPILED_HEADER = pch.hpp

INCLUDEPATH += ../dbus-model

interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
interface.header_flags = -i ../dbus-model/configurationparameter.hpp -i ../dbus-model/keyedmessage.hpp

DBUS_INTERFACES += interface

SOURCES += main.cpp\
    maindialog.cpp \
    configmodel.cpp \
    ../dbus-model/configurationparameter.cpp \
    ../dbus-model/keyedmessage.cpp \
    parameterchangelistener.cpp

HEADERS  += maindialog.hpp \
    pch.hpp \
    configmodel.hpp \
    ../dbus-model/configurationparameter.hpp \
    ../dbus-model/keyedmessage.hpp \
    parameterchangelistener.hpp

FORMS    += maindialog.ui
