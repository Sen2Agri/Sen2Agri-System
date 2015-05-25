QT += core sql dbus
QT -= gui

TARGET = sen2agri-persistence

DESTDIR = bin

CONFIG += console c++11 precompile_header
CONFIG -= app_bundle

PRECOMPILED_HEADER = pch.hpp

INCLUDEPATH += ../dbus-model

TEMPLATE = app

adaptor.files = ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml
adaptor.header_flags = -i ../dbus-model/configurationparameter.hpp \
                       -i ../dbus-model/keyedmessage.hpp \
                       -i ../dbus-model/archiverparameter.hpp

DBUS_ADAPTORS += adaptor

SOURCES += main.cpp \
    persistencemanager.cpp \
    dbprovider.cpp \
    ../dbus-model/configurationparameter.cpp \
    sql_error.cpp \
    qsqldatabaseraii.cpp \
    ../dbus-model/keyedmessage.cpp \
    ../dbus-model/archiverparameter.cpp \
    persistencemanagerdbprovider.cpp \
    settings.cpp \
    logger.cpp

DISTFILES += \
# install to /usr/share/dbus-1/interfaces/org.esa.sen2agri.persistenceManager.xml
    ../dbus-interfaces/org.esa.sen2agri.persistenceManager.xml \
# install to /etc/dbus-1/system.d/org.esa.sen2agri.persistence-manager.conf
    dist/org.esa.sen2agri.persistence-manager.conf \
# install to /usr/share/dbus-1/system-services/org.esa.sen2agri.persistence-manager.service
    dist/org.esa.sen2agri.persistence-manager.service \
# install to /etc/systemd/system/sen2agri-persistence.service
    dist/sen2agri-persistence.service \
# install to /etc/sen2agri/sen2agri-persistence.conf
    dist/sen2agri-persistence.conf

HEADERS += \
    persistencemanager.hpp \
    dbprovider.hpp \
    ../dbus-model/configurationparameter.hpp \
    sql_error.hpp \
    pch.hpp \
    qsqldatabaseraii.hpp \
    asyncdbustask.hpp \
    ../dbus-model/keyedmessage.hpp \
    ../dbus-model/archiverparameter.hpp \
    persistencemanagerdbprovider.hpp \
    settings.hpp \
    make_unique.hpp \
    logger.hpp
