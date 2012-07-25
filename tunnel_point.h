#pragma once
#include "network.h"

struct tunnel_point
{
    enum protocol_type {pt_udp, pt_tcp};

    tunnel_point(protocol_type type, network::endpoint const& sock_address)
        : type          (type   )
        , sock_address  (sock_address)
    {
    }

    protocol_type       type;
    network::endpoint   sock_address;
};

bool operator<(tunnel_point const& lhs, tunnel_point const& rhs)
{
    return
        lhs.type                 < rhs.type                                                                     ||
        lhs.type                == rhs.type                 && lhs.sock_address.addr < rhs.sock_address.addr    ||
        lhs.sock_address.addr   == rhs.sock_address.addr    && lhs.sock_address.port < rhs.sock_address.port;
}

struct receiver
{
    virtual tunnel_point const& point() = 0;

    virtual ~receiver(){}
};
typedef shared_ptr<receiver> receiver_ptr;


struct sender
{
    virtual tunnel_point const& point() = 0;

    virtual void send(const void* data, size_t size) = 0;
    virtual void ~sender(){}
};
typedef shared_ptr<sender> sender_ptr;

namespace std
{
    template<>
    struct less<receiver_ptr>
            : binary_function<receiver_ptr, receiver_ptr, bool>
    {
        bool operator()(receiver_ptr const& lhs, receiver_ptr const& rhs) const { return lhs->point < rhs->point; }
    };

    template<>
    struct less<sender_ptr>
            : binary_function<sender_ptr, sender_ptr, bool>
    {
        bool operator()(sender_ptr const& lhs, sender_ptr const& rhs) const { return lhs->point < rhs->point; }
    };
} // namespace std



struct udp_sender
    : sender
{
    udp_sender(io_service& io, network::endpoint const& remote_server)
        : sock_(io, none, remote_server, network::on_receive_f(0), trace_error)
        , point_(tunnel_point::pt_udp, remote_server)
    {
    }

    tunnel_point const& point()
    {
        return point_;
    }

    void send(const void* data, size_t size)
    {
        sock_.send(data, size);
    }

private:
    network::udp_socket sock_;
    tunnel_point        point_;
};

struct tcp_sender
    : sender
{
    tcp_sender(io_service& io, network::endpoint const& remote_server)
        : point_(tunnel_point::pt_tcp, remote_server)
    {
        start_connect(io, remote_server);
    }

    tunnel_point const& point()
    {
        return point_;
    }

    void send(const void* data, size_t size)
    {
        if (sock_)
            sock_.send(data, size);
    }

private:
    void start_connect(io_service& io, network::endpoint const& remote_server)
    {
        network::connect(io, remote_server, bind(&tcp_sender::on_connected, this, _1), trace_error);
    }

    void on_disconnected()
    {
        sock_.reset();
        cout << ""

    }

    void on_connected(tcp::socket& sock)
    {
        sock_ = in_place(ref(sock), network::on_receive_f(0), bind(&tcp_sender::on_disconnected), trace_error);
    }

private:
    tunnel_point point_;
    optional<network::tcp_socket> sock_;
};





