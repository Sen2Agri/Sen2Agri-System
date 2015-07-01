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
dbus_interface.header_flags = -i ../../../dbus-model/model.hpp

DBUS_INTERFACES += dbus_interface


adaptor.files = ../../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
adaptor.header_flags = -i ../../../dbus-model/model.hpp

DBUS_ADAPTORS += adaptor

INCLUDEPATH += ../../../dbus-model ../../../Optional

SOURCES += main.cpp \
    persistencemanager.cpp \
    ../../../dbus-model/model.cpp \
    ApplicationClosingListener.cpp

HEADERS += \
    ApplicationClosingListener.h \
    persistencemanager.h \
    ../../../dbus-model/model.hpp

target.path = ../../dist
INSTALLS += target

OTHER_FILES += \
    ../../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml \
    ../../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
