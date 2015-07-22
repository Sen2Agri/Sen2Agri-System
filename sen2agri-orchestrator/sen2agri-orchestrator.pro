include(../common.pri)

QT -= gui
QT += core dbus

TARGET = sen2agri-orchestrator

DESTDIR = bin

CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

adaptor.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml
adaptor.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

persistence_manager_interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
persistence_manager_interface.header_flags = -i ../sen2agri-common/model.hpp

processors_executor_interface.files = ../dbus-interfaces/org.esa.sen2agri.processorsExecutor.xml
processors_executor_interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += persistence_manager_interface processors_executor_interface

SOURCES += main.cpp \
    orchestrator.cpp \
    orchestratorworker.cpp \
    eventprocessingcontext.cpp \
    processorhandler.cpp \
    processor/dummyprocessorhandler.cpp

HEADERS += \
    pch.hpp \
    orchestrator.hpp \
    orchestratorworker.hpp \
    eventprocessingcontext.hpp \
    processorhandler.hpp \
    processor/dummyprocessorhandler.hpp

DISTFILES += \
    ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml \
    dist/org.esa.sen2agri.orchestrator.conf \
    dist/org.esa.sen2agri.orchestrator.service \
    dist/sen2agri-orchestrator.service

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

interface.path = /usr/share/dbus-1/interfaces
interface.files = ../dbus-interfaces/org.esa.sen2agri.orchestrator.xml

dbus-policy.path = /etc/dbus-1/system.d
dbus-policy.files = dist/org.esa.sen2agri.orchestrator.conf

dbus-service.path = /usr/share/dbus-1/system-services
dbus-service.files = dist/org.esa.sen2agri.orchestrator.service

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-orchestrator.service

INSTALLS += target interface dbus-policy dbus-service systemd-service
