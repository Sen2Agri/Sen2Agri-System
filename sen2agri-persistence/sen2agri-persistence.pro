include(../common.pri)

QT += core sql dbus
QT -= gui

TARGET = sen2agri-persistence

DESTDIR = bin

CONFIG -= app_bundle

INCLUDEPATH += ../Optional

TEMPLATE = app

adaptor.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
adaptor.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

SOURCES += main.cpp \
    persistencemanager.cpp \
    dbprovider.cpp \
    sql_error.cpp \
    sqldatabaseraii.cpp \
    persistencemanagerdbprovider.cpp \
    settings.cpp \
    serializedevent.cpp

DISTFILES += \
    ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml \
    dist/org.esa.sen2agri.persistence-manager.conf \
    dist/org.esa.sen2agri.persistence-manager.service \
    dist/sen2agri-persistence.service \
    dist/sen2agri-persistence.conf

HEADERS += \
    persistencemanager.hpp \
    dbprovider.hpp \
    sql_error.hpp \
    pch.hpp \
    sqldatabaseraii.hpp \
    asyncdbustask.hpp \
    persistencemanagerdbprovider.hpp \
    settings.hpp \
    serializedevent.hpp

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

interface.path = /usr/share/dbus-1/interfaces
interface.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml

dbus-policy.path = /etc/dbus-1/system.d
dbus-policy.files = dist/org.esa.sen2agri.persistence-manager.conf

dbus-service.path = /usr/share/dbus-1/system-services
dbus-service.files = dist/org.esa.sen2agri.persistence-manager.service

systemd-service.path = /usr/lib/systemd/system
systemd-service.files = dist/sen2agri-persistence.service

conf.path = /etc/sen2agri
conf.files = dist/sen2agri-persistence.conf

INSTALLS += target interface dbus-policy dbus-service systemd-service conf
