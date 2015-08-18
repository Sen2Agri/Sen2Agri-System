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
    simpleudpinfosclient.cpp \
    simpletcpinfosclient.cpp

HEADERS += \
    abstractexecinfosprotclient.h \
    applicationclosinglistener.h \
    commandinvoker.h \
    icommandinvokerlistener.h \
    processorwrapper.h \
    simpleudpinfosclient.h \
    pch.hpp \
    simpletcpinfosclient.h

target.path = /usr/bin

INSTALLS += target

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
