#pragma once
#include "net_common.h"
#include "tunnel_tcp_service.h"
#include "tunnel_udp_service.h"

struct tunnel
{
    void create_udp_receiver(network::endpoint const& local_bind, network::endpoint const& remote_server, string remote_tcp_point = "");
    void create_tcp_sender(bool client, network::endpoint const& point, posix_time::time_duration const& reconnect_timeout = posix_time::seconds(2));

    void run();

private:
    typedef map<network::endpoint, udp_receiver_ptr>    receivers_t;

private:
    io_service io_;

private:
    receivers_t     receivers_;
    tcp_sender_ptr  sender_;
};
