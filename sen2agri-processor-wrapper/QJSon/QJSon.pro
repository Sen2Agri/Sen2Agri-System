#-------------------------------------------------
#
# Project created by QtCreator 2015-05-28T18:30:20
#
#-------------------------------------------------

QT       -= gui

TARGET = QJSon
TEMPLATE = lib

DEFINES += QJSON_MAKEDLL

DEFINES += QT_SHARED

SOURCES += \
    src/json_scanner.cpp \
    src/parser.cpp \
    src/parserrunnable.cpp \
    src/qobjecthelper.cpp \
    src/serializer.cpp \
    src/serializerrunnable.cpp \
    src/json_parser.cc \
    src/json_scanner.cc

HEADERS += \
    src/FlexLexer.h \
    src/json_parser.hh \
    src/json_scanner.h \
    src/location.hh \
    src/parser.h \
    src/parser_p.h \
    src/parserrunnable.h \
    src/position.hh \
    src/qjson_debug.h \
    src/qjson_export.h \
    src/qobjecthelper.h \
    src/serializer.h \
    src/serializerrunnable.h \
    src/stack.hh \
    include/QJson/Parser \
    include/QJson/QObjectHelper \
    include/QJson/Serializer

win32 {
    target.path = ../dist
    INSTALLS += target
}

unix {
    # if the file system is readonly (mounted from a windows partition) just copy the file as the
    # symbolic link will not work
    target.path = ../dist
    INSTALLS += target

    extra_install.path = ../dist
    extra_install.extra = if ! [ -f ../../dist/libQJSon.so ]; then cp -f ../../dist/libQJSon.so.1.0.0 ../../dist/libQJSon.so && cp -f ../../dist/libQJSon.so.1.0.0 ../../dist/libQJSon.so.1; fi
    INSTALLS += extra_install
}

DISTFILES += \
    src/json_parser.yy \
    src/json_scanner.yy
