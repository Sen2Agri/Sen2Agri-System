include(../common.pri)

QT -= gui
QT += dbus sql

TARGET = sen2agri-persistence
TEMPLATE = lib

CONFIG += staticlib

INCLUDEPATH += ../Optional

SOURCES += \
    persistencemanager.cpp \
    dbprovider.cpp \
    sql_error.cpp \
    sqldatabaseraii.cpp \
    settings.cpp \
    serializedevent.cpp \
    credential_utils.cpp

HEADERS += \
    persistencemanager.hpp \
    dbprovider.hpp \
    sql_error.hpp \
    pch.hpp \
    sqldatabaseraii.hpp \
    asyncdbustask.hpp \
    settings.hpp \
    serializedevent.hpp \
    credential_utils.hpp

DISTFILES += dist/sen2agri-persistence.conf

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

conf.path = /etc/sen2agri
conf.files = dist/sen2agri-persistence.conf

INSTALLS += conf
