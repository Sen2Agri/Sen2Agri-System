include(../common.pri)

QT       += core dbus sql

QT       -= gui

TARGET = sen2agri-archiver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DESTDIR = bin

INCLUDEPATH += ../Optional

SOURCES += main.cpp \
    archivermanager.cpp

HEADERS += \
    archivermanager.hpp \
    pch.hpp

DISTFILES +=

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence

INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
