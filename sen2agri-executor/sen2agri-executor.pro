include(../common.pri)

QT       += core dbus network
QT       -= gui

TARGET = sen2agri-executor

DESTDIR = bin

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += QT_SHARED

TEMPLATE = app

INCLUDEPATH += ../Optional

dbus_interface.files = ../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
dbus_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += dbus_interface

dbus_interface2.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
dbus_interface2.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += dbus_interface2

dbus_interface3.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
dbus_interface3.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += dbus_interface3

SOURCES += main.cpp \
    abstractexecinfosprotsrv.cpp \
    execinfosprotsrvfactory.cpp \
    orchestratorrequestshandler.cpp \
    persistenceitfmodule.cpp \
    ressourcemanageritf.cpp \
    simpleudpexecinfosprotsrv.cpp \
    commandinvoker.cpp \
    slurmsacctresultparser.cpp \
    processorexecutioninfos.cpp \
    processorwrapperfactory.cpp \
    configurationmgr.cpp \
    requestparamsbase.cpp \
    requestparamscanceltasks.cpp \
    requestparamssubmitsteps.cpp \
    requestparamsexecutioninfos.cpp \
    orchestratorclient.cpp

HEADERS += \
    abstractexecinfosprotsrv.h \
    execinfosprotsrvfactory.h \
    orchestratorrequestshandler.h \
    persistenceitfmodule.h \
    ressourcemanageritf.h \
    simpleudpexecinfosprotsrv.h \
    iprocessorwrappermsgslistener.h \
    commandinvoker.h \
    slurmsacctresultparser.h \
    processorexecutioninfos.h \
    processorwrapperfactory.h \
    configurationmgr.h \
    pch.hpp \
    requestparamsbase.h \
    requestparamscanceltasks.h \
    requestparamssubmitsteps.h \
    requestparamsexecutioninfos.h \
    orchestratorclient.h

OTHER_FILES += \
    ../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml \
    ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml \
    dist/org.esa.sen2agri.processorsExecutor.conf \
    dist/org.esa.sen2agri.processorsExecutor.service \
    dist/sen2agri-executor.service

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

INSTALLS += target interface dbus-policy dbus-service systemd-service conf
