#pragma once
#include "tunnel_common.h"
#include "udp_service.h"

struct udp_sender
    : noncopyable
{
    udp_sender(io_service& io);
    void send(const void* data, size_t size);

private:
    network::udp_socket sock_;
};

struct udp_receiver
    : noncopyable
{
    udp_receiver(io_service& io, network::endpoint const& local_bind, network::endpoint const& remote_server, tcp_sender_ptr sndr, string remote_tcp_point = "");
    void on_receive(const void* data, size_t size);

private:
    network::udp_socket sock_;
    tcp_sender_ptr      sender_;
    network::endpoint   remote_server_;
    string              remote_tcp_point_;
};
