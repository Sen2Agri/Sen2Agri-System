#-------------------------------------------------
#
# Project created by QtCreator 2015-05-22T13:47:33
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

DEFINES += QT_SHARED

TARGET = OrchestratorSimulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += c++11 precompile_header

dbus_interface.files = ../../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
dbus_interface.header_flags = -i ../../../sen2agri-common/model.hpp

DBUS_INTERFACES += dbus_interface

INCLUDEPATH += ../../../sen2agri-common ../../../Optional

SOURCES += main.cpp \
    ../../../sen2agri-common/model.cpp \
    ../../../sen2agri-common/json_conversions.cpp \
    ApplicationClosingListener.cpp \
    simulator.cpp

HEADERS += \
    ApplicationClosingListener.h \
    ../../../sen2agri-common/model.hpp \
    ../../../sen2agri-common/json_conversions.hpp \
    simulator.h

target.path = ../../dist
INSTALLS += target interface dbus-policy dbus-service systemd-service conf

OTHER_FILES += \
    ../../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
