include(../common.pri)

QT -= gui
QT += core dbus sql

TARGET = sen2agri-orchestrator

DESTDIR = bin

CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

adaptor.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
adaptor.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

processors_executor_interface.files = ../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
processors_executor_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += processors_executor_interface

SOURCES += main.cpp \
    orchestrator.cpp \
    orchestratorworker.cpp \
    eventprocessingcontext.cpp \
    processorhandler.cpp \
    processor/croptypehandler.cpp \
    processor/cropmaskhandler.cpp \
    tasktosubmit.cpp \
    processor/compositehandler.cpp \
    processor/lairetrievalhandler.cpp \
    processor/lairetrievalhandler_l3b.cpp \
    processor/lairetrievalhandler_l3c.cpp \
    processor/maccshdrmeananglesreader.cpp \
    processor/phenondvihandler.cpp \
    processorhandlerhelper.cpp \
    schedulingcontext.cpp \
    processor/ndvihandler.cpp \
    processor/lairetrievalhandler_l3b_new.cpp \
    processor/s4c_croptypehandler.cpp \
    processor/agricpracticeshandler.cpp \
    processor/grasslandmowinghandler.cpp

HEADERS += \
    pch.hpp \
    orchestrator.hpp \
    orchestratorworker.hpp \
    eventprocessingcontext.hpp \
    processorhandler.hpp \
    processor/croptypehandler.hpp \
    processor/cropmaskhandler.hpp \
    tasktosubmit.hpp \
    processor/compositehandler.hpp \
    processor/lairetrievalhandler.hpp \
    processor/lairetrievalhandler_l3b.hpp \
    processor/lairetrievalhandler_l3c.hpp \
    processor/maccshdrmeananglesreader.hpp \
    processor/phenondvihandler.hpp \
    processorhandlerhelper.h \
    schedulingcontext.h \
    processor/ndvihandler.hpp \
    processor/lairetrievalhandler_l3b_new.hpp \
    processor/s4c_croptypehandler.hpp \
    processor/agricpracticeshandler.hpp \
    processor/grasslandmowinghandler.hpp

DISTFILES += \
    ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml \
    dist/org.esa.sen2agri.orchestrator.conf \
    dist/org.esa.sen2agri.orchestrator.service \
    dist/sen2agri-orchestrator.service

target.path = /usr/bin

interface.path = /usr/share/dbus-1/interfaces
interface.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml

dbus-policy.path = /etc/dbus-1/system.d
dbus-policy.files = dist/org.esa.sen2agri.orchestrator.conf

dbus-service.path = /usr/share/dbus-1/system-services
dbus-service.files = dist/org.esa.sen2agri.orchestrator.service

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-orchestrator.service

slurm-override.path = /etc/systemd/system/slurmd.service.d
slurm-override.files = dist/override.conf

INSTALLS += target interface dbus-policy dbus-service systemd-service slurm-override

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence

INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
