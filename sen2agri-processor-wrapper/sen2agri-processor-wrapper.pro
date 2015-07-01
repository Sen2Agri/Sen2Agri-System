QT += core network
QT -= gui

TARGET = sen2agri-processor-wrapper

DESTDIR = bin

CONFIG += c++11 precompile_header
CONFIG -= app_bundle

PRECOMPILED_HEADER = pch.hpp

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
