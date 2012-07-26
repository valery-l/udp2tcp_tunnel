#pragma once

#include "network.h"
#include "async_timer.h"

struct tcp_sender
{
    virtual void send(const void* data, size_t size, string remote_tcp_server) = 0;
    virtual ~tcp_sender(){}
};
typedef shared_ptr<tcp_sender> tcp_sender_ptr;
