#-------------------------------------------------
#
# Project created by QtCreator 2015-05-27T17:41:29
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = ProcessorWrapper
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    commandinvoker.cpp \
    processorwrapper.cpp \
    simpleudpinfosclient.cpp

HEADERS += \
    commandinvoker.h \
    processorwrapper.h \
    icommandinvokerlistener.h \
    simpleudpinfosclient.h \
    abstractexecinfosprotclient.h \
    applicationclosinglistener.h

target.path = ../dist
INSTALLS += target
