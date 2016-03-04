include(../common.pri)

QT -= gui
QT += core dbus sql

DESTDIR = bin

CONFIG += console
CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

TARGET = sen2agri-scheduler

SOURCES += main.cpp \
    taskloader.cpp \
    schedulerapp.cpp \
    taskplanner.cpp \
    orchestratorproxy.cpp \
    resourcereader.cpp \
    runestimator.cpp \
    databasetaskloader.cpp \
    dbusorchestratorproxy.cpp

#adaptor.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
#adaptor.header_flags = -i ../sen2agri-common/model.hpp

#DBUS_ADAPTORS += adaptor

orchestrator_interface.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
orchestrator_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += orchestrator_interface

DISTFILES += \
    dist/sen2agri-scheduler.service

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence
LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common


INCLUDEPATH += $$PWD/../sen2agri-common
INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

target.path = /usr/bin

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-scheduler.service

INSTALLS += target systemd-service

HEADERS += \
    pch.hpp \
    scheduledtask.hpp \
    taskloader.hpp \
    schedulerapp.hpp \
    taskplanner.hpp \
    resourcereader.hpp \
    orchestratorproxy.hpp \
    runestimator.hpp \
    databasetaskloader.hpp \
    dbusorchestratorproxy.hpp
