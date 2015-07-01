QT += core network dbus
QT -= gui

TARGET = sen2agri-http-listener

DESTDIR = bin

CONFIG += c++11 precompile_header
CONFIG -= app_bundle

PRECOMPILED_HEADER = pch.hpp

INCLUDEPATH += ../Optional

TEMPLATE = app

persistence_manager_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
persistence_manager_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += persistence_manager_interface

SOURCES += main.cpp \
    requestmapper.cpp \
    controller/dashboardcontroller.cpp

QTWEBAPP = -lQtWebApp
CONFIG(debug, debug|release) {
    QTWEBAPP = $$join(QTWEBAPP,,,d)
}

LIBS += -L$$OUT_PWD/../QtWebApp/ $$QTWEBAPP

INCLUDEPATH += $$PWD/../QtWebApp
DEPENDPATH += $$PWD/../QtWebApp

HEADERS += \
    controller/dashboardcontroller.hpp \
    requestmapper.hpp \
    pch.hpp

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
