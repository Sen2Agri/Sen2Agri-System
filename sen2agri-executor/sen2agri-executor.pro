#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T18:21:01
#
#-------------------------------------------------

QT       += core dbus network
QT       -= gui

TARGET = ProcessorsExecutor

DESTDIR = bin

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += QT_SHARED

TEMPLATE = app

CONFIG += c++11 precompile_header

INCLUDEPATH += ../Optional

dbus_interface.files = ../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
dbus_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += dbus_interface

dbus_interface2.files = ../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
dbus_interface2.header_flags = -i ../sen2agri-common/model.hpp

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
    logger.cpp

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
    logger.h

OTHER_FILES += \
    ../../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml \
    ../../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml

#target.path = ../dist
#INSTALLS += target

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
