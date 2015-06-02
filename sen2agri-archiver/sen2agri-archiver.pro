#-------------------------------------------------
#
# Project created by QtCreator 2015-05-26T14:51:11
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

TARGET = sen2agri-archiver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = bin

CONFIG += c++11 precompile_header

INCLUDEPATH += ../dbus-model ../Optional

dbus_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
dbus_interface.header_flags = -i ../dbus-model/model.hpp

DBUS_INTERFACES += dbus_interface

SOURCES += main.cpp \
    archivermanager.cpp \
    ../dbus-model/model.cpp

HEADERS += \
    archivermanager.hpp \
    ../dbus-model/model.hpp

DISTFILES +=
