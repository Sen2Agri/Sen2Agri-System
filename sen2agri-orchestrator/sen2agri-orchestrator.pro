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
    executioncontextbase.cpp \
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
    processor/grasslandmowinghandler.cpp \
    processor/s4c_utils.cpp

HEADERS += \
    pch.hpp \
    orchestrator.hpp \
    orchestratorworker.hpp \
    executioncontextbase.hpp \
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
    processor/grasslandmowinghandler.hpp \
    processor/s4c_utils.hpp

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

slurmd-override.path = /etc/systemd/system/slurmd.service.d
slurmd-override.files = dist/slurmd.override.conf

slurmctld-override.path = /etc/systemd/system/slurmctld.service.d
slurmctld-override.files = dist/slurmctld.override.conf

slurmdbd-override.path = /etc/systemd/system/slurmdbd.service.d
slurmdbd-override.files = dist/slurmdbd.override.conf

INSTALLS += target interface dbus-policy dbus-service systemd-service slurmd-override slurmctld-override slurmdbd-override

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence

INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a
