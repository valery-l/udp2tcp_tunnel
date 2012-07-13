TEMPLATE = app

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
    connecting.h

