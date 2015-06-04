#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T18:21:01
#
#-------------------------------------------------

QT       += core
QT       += dbus
QT       += network
QT       -= gui

TARGET = ProcessorsExecutor
CONFIG   += console
CONFIG   -= app_bundle

DEFINES += QT_SHARED

TEMPLATE = app


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
    ProcessorsExecutorAdapter.cpp \
    ProcessorsExecutorProxy.cpp

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
    ProcessorsExecutorAdapter.h \
    ProcessorsExecutorProxy.h

OTHER_FILES +=

target.path = ../dist
INSTALLS += target

unix|win32: LIBS += -L$$PWD/../dist/ -lQJSon

INCLUDEPATH += $$PWD/../QJSon/include
DEPENDPATH += $$PWD/../QJSon/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../dist/ -lQJSon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../dist/ -lQJSon
else:unix: LIBS += -L$$PWD/../dist/ -lQJSon

