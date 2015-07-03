include(../common.pri)

QT       += core gui widgets dbus

TARGET = sen2agri-config
TEMPLATE = app

DESTDIR = bin

INCLUDEPATH += ../Optional

interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += interface

SOURCES += main.cpp\
    maindialog.cpp \
    configmodel.cpp \
    parameterchangelistener.cpp \
    parameterkey.cpp

HEADERS  += maindialog.hpp \
    pch.hpp \
    configmodel.hpp \
    parameterchangelistener.hpp \
    parameterkey.hpp

FORMS    += maindialog.ui

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
