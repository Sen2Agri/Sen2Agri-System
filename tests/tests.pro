include(../common.pri)

QT += core testlib dbus
QT -= gui

TARGET = tests

DESTDIR = bin

CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../Optional

SOURCES += main.cpp \
    testqstring.cpp \
    serialization.cpp \
    reflector.cpp \
    serialization-ops.cpp

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

adaptor.files = reflector.xml
adaptor.header_flags = -i ../sen2agri-common/model.hpp

DBUS_ADAPTORS += adaptor

interface.files = reflector.xml
interface.header_flags = -i ../sen2agri-common/model.hpp

DBUS_INTERFACES += interface

HEADERS += \
    testqstring.hpp \
    serialization.hpp \
    reflector.hpp \
    serialization-ops.hpp \
    pch.hpp

DISTFILES += \
    reflector.xml
