#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T18:21:01
#
#-------------------------------------------------

QT       += core dbus network
QT       -= gui

TARGET = ProcessorsExecutor
CONFIG   += console
CONFIG   -= app_bundle

DEFINES += QT_SHARED

TEMPLATE = app

CONFIG += c++11 precompile_header

INCLUDEPATH += ../../dbus-model ../../Optional

dbus_interface.files = ../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
dbus_interface.header_flags = -i ../../dbus-model/model.hpp

DBUS_ADAPTORS += dbus_interface

dbus_interface2.files = ../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
dbus_interface2.header_flags = -i ../../dbus-model/model.hpp

DBUS_INTERFACES += dbus_interface2


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
    logger.cpp \
    ../../dbus-model/model.cpp

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
    logger.h \
    ../../dbus-model/model.hpp

OTHER_FILES += \
    ../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml \
    ../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml

target.path = ../dist
INSTALLS += target


