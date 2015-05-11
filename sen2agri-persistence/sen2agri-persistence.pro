QT += core sql dbus
QT -= gui

TARGET = sen2agri-persistence

DESTDIR = bin

CONFIG += console c++11 precompile_header
CONFIG -= app_bundle

PRECOMPILED_HEADER = pch.hpp

TEMPLATE = app

adaptor.files = org.esa.sen2agri.persistenceManager.xml
adaptor.header_flags = -i configurationparameter.hpp

DBUS_ADAPTORS += adaptor

SOURCES += main.cpp \
    persistencemanager.cpp \
    dbprovider.cpp \
    configurationparameter.cpp \
    sql_error.cpp \
    qsqldatabaseraii.cpp \
    keyedmessage.cpp \
    persistencemanagerdbprovider.cpp

DISTFILES += \
    org.esa.sen2agri.persistenceManager.xml

HEADERS += \
    persistencemanager.hpp \
    dbprovider.hpp \
    configurationparameter.hpp \
    sql_error.hpp \
    pch.hpp \
    qsqldatabaseraii.hpp \
    asyncdbustask.hpp \
    keyedmessage.hpp \
    persistencemanagerdbprovider.hpp
