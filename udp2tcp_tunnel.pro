TEMPLATE = app

BIN_PATH = ../bin/$$BIN_NAME

release : BUILD_PATH = $$join(BIN_PATH,,,"/release")
debug   : BUILD_PATH = $$join(BIN_PATH,,,"/debug")

debug{
    message("Debug")
}

release{
    message("Release")
}

DESTDIR     = $$BUILD_PATH
OBJECTS_DIR = $$BUILD_PATH/misc
MOC_DIR     = $$BUILD_PATH/misc
UI_DIR      = $$BUILD_PATH/misc
RCC_DIR     = $$BUILD_PATH/misc

CONFIG += console
CONFIG -= qt


LIBS += -lboost_system -lpthread -lboost_thread
QMAKE_CXXFLAGS += -std=c++0x
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++

SOURCES += main.cpp \
    udp_server.cpp \
    udp_client.cpp \
    tcp_client.cpp \
    tcp_server.cpp \
    connecting.cpp

HEADERS += \
    common.h \
    tcp_service.h \
    asio_helper.h \
    connecting.h \
    c++_utils.h

