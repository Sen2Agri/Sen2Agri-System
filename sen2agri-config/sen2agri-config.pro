QT       += core gui widgets dbus

TARGET = sen2agri-config
TEMPLATE = app

DESTDIR = bin

CONFIG += c++11 precompile_header

PRECOMPILED_HEADER = pch.hpp

INCLUDEPATH += ../dbus-model ../Optional

interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
interface.header_flags = -i ../dbus-model/model.hpp

DBUS_INTERFACES += interface

SOURCES += main.cpp\
    maindialog.cpp \
    configmodel.cpp \
    parameterchangelistener.cpp \
    parameterkey.cpp \
    ../dbus-model/model.cpp

HEADERS  += maindialog.hpp \
    pch.hpp \
    configmodel.hpp \
    parameterchangelistener.hpp \
    parameterkey.hpp \
    ../dbus-model/model.hpp

FORMS    += maindialog.ui
