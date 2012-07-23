TEMPLATE = app

message($$CONFIG)

BIN_PATH = ../bin

#CONFIG(debug)   : BUILD_PATH = $$join(BIN_PATH,,,"/debug")
#CONFIG(release) : BUILD_PATH = $$join(BIN_PATH,,,"/release")

debug{
    BUILD_PATH = $$join(BIN_PATH,,,"/debug")
    message("Debug")
}

release{
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
QMAKE_CXXFLAGS += -std=c++0x -g3
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++

SOURCES += main.cpp \
    connecting.cpp \
    tcp_service.cpp \
    udp_service.cpp \
    client.cpp \
    server.cpp \
    __udp_client.cpp \
    __udp_server.cpp

HEADERS += \
    common.h \
    tcp_service.h \
    asio_helper.h \
    connecting.h \
    cpp_utils.h \
    underlying_transport_impl.h \
    net_common.h \
    udp_service.h

