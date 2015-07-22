include(../common.pri)

QT += core network dbus
QT -= gui

TARGET = sen2agri-http-listener

DESTDIR = bin

CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

persistence_manager_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
persistence_manager_interface.header_flags = -i ../sen2agri-common/model.hpp

orchestrator_interface.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
orchestrator_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += persistence_manager_interface orchestrator_interface

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

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-http-listener.service

INSTALLS += target systemd-service
