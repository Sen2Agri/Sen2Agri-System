#-------------------------------------------------
#
# Project created by QtCreator 2015-06-30T16:07:58
#
#-------------------------------------------------

QT       += core network dbus

QT       -= gui

TARGET = sen2agri-http-listener
CONFIG   += c++11 console
CONFIG   -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

persistence_manager_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
persistence_manager_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += persistence_manager_interface

SOURCES += main.cpp \
    requestmapper.cpp \
    controller/dashboardcontroller.cpp

LIBS += -L$$OUT_PWD/../QtWebApp/ -lQtWebAppd

INCLUDEPATH += $$PWD/../QtWebApp
DEPENDPATH += $$PWD/../QtWebApp

HEADERS += \
    requestmapper.h \
    controller/dashboardcontroller.h

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
