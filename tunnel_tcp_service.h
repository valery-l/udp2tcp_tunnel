#pragma once
#include "tunnel_common.h"
#include "tunnel_udp_service.h"
#include "tcp_service.h"

struct tcp_client
    : tcp_sender
{
    tcp_client(io_service& io, network::endpoint const& remote_server, posix_time::time_duration const& reconnect_timeout = posix_time::seconds(2));
    void send(const void* data, size_t size, string);

private:

    void start_connect(io_service& io, network::endpoint const& remote_server);

    void on_receive     (const void* data, size_t size);
    void on_disconnected(string reason);
    void on_connected   (tcp::socket& sock, network::endpoint const& remote_peer);

private:
    optional<network::tcp_socket>       sock_;
    optional<network::async_connector>  connector_;

private:
    udp_sender udp_sender_;

private:
    async_timer                 reconnect_timer_;
    posix_time::time_duration   reconnect_timeout_;
};


struct tcp_server
    : tcp_sender
{
    tcp_server(io_service& io, network::endpoint const& local_bind);
    void send(const void* data, size_t size, string client);

private:
    void on_accept      (tcp::socket& sock, network::endpoint const& remote_peer);
    void on_receive     (const void* data, size_t size);
    void on_disconnected(string client);

private:
    udp_sender udp_sender_;

private:
    network::async_acceptor acceptor_;

private:
    typedef
        map<string, shared_ptr<network::tcp_socket> >
        clients_t;

private:
    clients_t clients_;
};
