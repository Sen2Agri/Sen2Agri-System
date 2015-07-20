include(../common.pri)

QT += core network
QT -= gui

TARGET = sen2agri-monitor-agent

DESTDIR = bin

CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    stats.cpp \
    monitor.cpp \
    settings.cpp

HEADERS += \
    pch.hpp \
    stats.hpp \
    monitor.hpp \
    settings.hpp

DISTFILES += dist/sen2agri-monitor-agent.conf

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
