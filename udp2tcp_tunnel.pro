TEMPLATE = app

CONFIG += debug_and_release

BIN_PATH = ../bin/$$BIN_NAME

#CONFIG(debug)   : BUILD_PATH = $$join(BIN_PATH,,,"/debug")
#CONFIG(release) : BUILD_PATH = $$join(BIN_PATH,,,"/release")

CONFIG(debug){
    BUILD_PATH = $$join(BIN_PATH,,,"/debug")
    message("Debug")
}

CONFIG(release){
    BUILD_PATH = $$join(BIN_PATH,,,"/release")
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

