include(../common.pri)

QT += core network dbus sql
QT -= gui

TARGET = sen2agri-http-listener

DESTDIR = bin

CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

orchestrator_interface.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
orchestrator_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += orchestrator_interface

SOURCES += main.cpp \
    requestmapper.cpp \
    controller/dashboardcontroller.cpp \
    controller/statisticscontroller.cpp

HEADERS += \
    controller/dashboardcontroller.hpp \
    controller/statisticscontroller.hpp \
    requestmapper.hpp \
    pch.hpp

DISTFILES += dist/sen2agri-http-listener.service

QTWEBAPP = -lQtWebApp
CONFIG(debug, debug|release) {
    QTWEBAPP = $$join(QTWEBAPP,,,d)
}

LIBS += -L$$OUT_PWD/../QtWebApp/ $$QTWEBAPP

INCLUDEPATH += $$PWD/../QtWebApp
DEPENDPATH += $$PWD/../QtWebApp

PRE_TARGETDEPS += $$OUT_PWD/../QtWebApp/libQtWebAppd.so

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence

INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-http-listener.service

INSTALLS += target systemd-service

