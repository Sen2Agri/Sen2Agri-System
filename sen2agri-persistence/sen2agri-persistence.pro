QT += core sql dbus
QT -= gui

TARGET = sen2agri-persistence

CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    persistencemanager.cpp \
    dbprovider.cpp \
    configurationparameter.cpp \
    sql_error.cpp

DISTFILES += \
    org.esa.sen2agri.persistenceManager.xml

HEADERS += \
    persistencemanager.hpp \
    dbprovider.hpp \
    configurationparameter.hpp \
    sql_error.hpp
