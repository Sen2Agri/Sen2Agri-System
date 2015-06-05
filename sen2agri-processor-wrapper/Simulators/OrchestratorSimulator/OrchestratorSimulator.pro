#-------------------------------------------------
#
# Project created by QtCreator 2015-05-22T13:47:33
#
#-------------------------------------------------

QT       += core
QT       += dbus

QT       -= gui

DEFINES += QT_SHARED

TARGET = OrchestratorSimulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ProcessorsExecutorProxy.cpp

HEADERS += \
    ProcessorsExecutorProxy.h \
    ApplicationClosingListener.h

target.path = ../../dist
INSTALLS += target
