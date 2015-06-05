#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T18:17:27
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = SlurmSrunSimulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    commandinvoker.cpp

HEADERS += \
    commandinvoker.h \
    icommandinvokerlistener.h \
    applicationclosinglistener.h

target.path = ../../dist
INSTALLS += target
