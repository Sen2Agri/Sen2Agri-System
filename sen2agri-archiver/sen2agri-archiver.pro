include(../common.pri)

QT       += core dbus

QT       -= gui

TARGET = sen2agri-archiver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DESTDIR = bin

INCLUDEPATH += ../Optional

dbus_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
dbus_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += dbus_interface

SOURCES += main.cpp \
    archivermanager.cpp

HEADERS += \
    archivermanager.hpp \
    pch.hpp \
    pch.hpp

DISTFILES +=

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
