include(../common.pri)

QT       += core gui widgets dbus sql concurrent

TARGET = sen2agri-config
TEMPLATE = app

DESTDIR = bin

INCLUDEPATH += ../Optional

SOURCES += main.cpp\
    maindialog.cpp \
    configmodel.cpp \
    parameterchangelistener.cpp \
    parameterkey.cpp

HEADERS  += maindialog.hpp \
    pch.hpp \
    configmodel.hpp \
    parameterchangelistener.hpp \
    parameterkey.hpp

FORMS    += maindialog.ui

LIBS += -L$$OUT_PWD/../sen2agri-persistence/ -lsen2agri-persistence

INCLUDEPATH += $$PWD/../sen2agri-persistence
DEPENDPATH += $$PWD/../sen2agri-persistence

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-persistence/libsen2agri-persistence.a

LIBS += -L$$OUT_PWD/../sen2agri-common/ -lsen2agri-common

INCLUDEPATH += $$PWD/../sen2agri-common
DEPENDPATH += $$PWD/../sen2agri-common

PRE_TARGETDEPS += $$OUT_PWD/../sen2agri-common/libsen2agri-common.a

target.path = /usr/bin

INSTALLS += target
