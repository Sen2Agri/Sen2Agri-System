include(../common.pri)

QT += core network
QT -= gui

TARGET = sen2agri-processor-wrapper

DESTDIR = bin

CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    commandinvoker.cpp \
    main.cpp \
    processorwrapper.cpp \
    simpleudpinfosclient.cpp

HEADERS += \
    abstractexecinfosprotclient.h \
    applicationclosinglistener.h \
    commandinvoker.h \
    icommandinvokerlistener.h \
    processorwrapper.h \
    simpleudpinfosclient.h \
    pch.hpp

target.path = /usr/bin

INSTALLS += target
