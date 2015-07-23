#-------------------------------------------------
#
# Project created by QtCreator 2015-07-22T14:25:12
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

DEFINES += QT_SHARED

TARGET = PersistenceMngSimulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


CONFIG += c++11 precompile_header

adaptor.files = ../../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
adaptor.header_flags = -i ../../../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

INCLUDEPATH += ../../../sen2agri-common ../../../Optional

SOURCES += main.cpp \
    persistencemanager.cpp \
    ../../../sen2agri-common/model.cpp \
    ../../../sen2agri-common/json_conversions.cpp \
    ApplicationClosingListener.cpp \
    simulator.cpp

HEADERS += \
    ApplicationClosingListener.h \
    persistencemanager.h \
    ../../../sen2agri-common/model.hpp \
    ../../../sen2agri-common/json_conversions.hpp \
    simulator.h

target.path = ../../dist
INSTALLS += target interface dbus-policy dbus-service systemd-service conf

OTHER_FILES += \
    ../../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
