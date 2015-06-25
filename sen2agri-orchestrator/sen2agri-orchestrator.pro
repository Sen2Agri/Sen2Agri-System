QT -= gui
QT += core dbus

TARGET = sen2agri-orchestrator

DESTDIR = bin

CONFIG += c++11 precompile_header
CONFIG -= app_bundle

PRECOMPILED_HEADER = pch.hpp

INCLUDEPATH += ../Optional

TEMPLATE = app

adaptor.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
adaptor.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += interface

SOURCES += main.cpp \
    orchestrator.cpp

DISTFILES += \
# install to /usr/share/dbus-1/interfaces/org.esa.sen2agri.orchestrator.xml
    ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml \
# install to /etc/dbus-1/system.d/org.esa.sen2agri.orchestrator.conf
    dist/org.esa.sen2agri.orchestrator.conf \
# install to /usr/share/dbus-1/system-services/org.esa.sen2agri.orchestrator.service
    dist/org.esa.sen2agri.orchestrator.service \
# install to /etc/systemd/system/sen2agri-orchestrator.service
    dist/sen2agri-orchestrator.service

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

HEADERS += \
    pch.hpp \
    orchestrator.hpp
