QT       += core gui widgets

TARGET = sen2agri-config
TEMPLATE = app

DESTDIR = bin

CONFIG += c++11 precompile_header

PRECOMPILED_HEADER = pch.hpp


SOURCES += main.cpp\
        maindialog.cpp \
    configmodel.cpp

HEADERS  += maindialog.hpp \
    pch.hpp \
    configmodel.hpp

FORMS    += maindialog.ui
