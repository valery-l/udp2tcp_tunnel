#pragma once
#include "network.h"
#include "async_timer.h"

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

inline std::ostream& operator<<(std::ostream& o, tunnel_point const& point)
{
    return o << (point.type == tunnel_point::pt_udp ? "udp" : "tcp") << ":" << point;
}

inline bool operator<(tunnel_point const& lhs, tunnel_point const& rhs)
{
    return
        lhs.type                 < rhs.type                                                                     ||
        lhs.type                == rhs.type                 && lhs.sock_address.addr < rhs.sock_address.addr    ||
        lhs.sock_address.addr   == rhs.sock_address.addr    && lhs.sock_address.port < rhs.sock_address.port;
}

struct tcp_sender
{
    virtual void send(const void* data, size_t size, string remote_tcp_server) = 0;
    virtual void ~sender(){}
};
typedef shared_ptr<tcp_sender> tcp_sender_ptr;

inline vector<char> unite_with_endpoint(const void* data, size_t size, network::endpoint const& point)
{
    vector<char> buf;
    buf.reserve(size + endpoint_serialize_size());

    point.serialize(back_inserter(buf));
    copy(back_inserter(buf), data, data + size);

    return buf;
}

struct udp_sender
    : noncopyable
{
    udp_sender(io_service& io, network::endpoint const& remote_server)
        : sock_ (io, none, remote_server, network::on_receive_f(0), trace_error)
        , point_(tunnel_point::pt_udp, remote_server)
    {
    }

    tunnel_point const& point() const
    {
        return point_;
    }

    void send(const void* data, size_t size)
    {
        size_t hdr_size = network::endpoint_serialize_size();
        assert(size >= hdr_size);

        const char* bytes = reinterpret_cast<const char*>(data);
        sock_.send(bytes + hdr_size, size - hdr_size, network::endpoint(bytes));
    }

private:
    network::udp_socket sock_;
    tunnel_point        point_;
};
typedef shared_ptr<udp_sender> udp_sender_ptr;
typedef std::set  <udp_sender> udp_senders_t;

namespace std
{
    template<>
    struct less<udp_sender_ptr>
            : binary_function<udp_sender_ptr, udp_sender_ptr, bool>
    {
        bool operator()(udp_sender_ptr const& lhs, udp_sender_ptr const& rhs) const { return lhs->point < rhs->point; }
    };

    template<>
    struct less<tcp_sender_ptr>
            : binary_function<tcp_sender_ptr, tcp_sender_ptr, bool>
    {
        bool operator()(tcp_sender_ptr const& lhs, tcp_sender_ptr const& rhs) const { return lhs->point < rhs->point; }
    };
} // namespace std


struct udp_receiver
    : noncopyable
{
    udp_receiver(io_service& io, network::endpoint const& local_bind, network::endpoint const& remote_server, tcp_sender_ptr sndr, string remote_tcp_point = "")
        : sock_             (io, local_bind, none, bind(&udp_receiver, this, _1, _2), trace_error)
        , sender_           (sndr)
        , remote_server_    (remote_server)
        , remote_tcp_point_ (remote_tcp_point)
    {
    }

    void on_receive(const void* data, size_t size)
    {
        vector<char> buf = unite_with_endpoint(data, size, remote_server_);
        sender_->send(&buf[0], buf.size(), remote_tcp_point_);
    }

    tunnel_point const& point() const
    {
        return point_;
    }

private:
    tunnel_point        point_;
    network::udp_socket sock_;
    tcp_sender_ptr      sender_;
    network::endpoint   remote_server_;
    string              remote_tcp_point_;
};

struct tcp_client
{
    tcp_sender(io_service& io, network::endpoint const& remote_server, posix_time::time_duration const& reconnect_timeout = posix_time::seconds(2))
        : point_            (tunnel_point::pt_tcp, remote_server)
        , reconnect_timer_  (io, bind(&tcp_sender::start_connect, this, ref(io), remote_server))
    {
        start_connect(io, remote_server);
    }

    tunnel_point const& point() const
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
        connector_ = in_place(ref(io), remote_server, bind(&tcp_sender::on_connected, this, _1), bind(&tcp_sender::on_disconnected, this, "connection refused"), trace_error);
    }

    void on_disconnected(string reason)
    {
        sock_.reset();
        cout << point_ << ": " << reason << endl;

        reconnect_timer_.wait(reconnect_timeout_);
    }

    void on_connected(tcp::socket& sock)
    {
        sock_ = in_place(ref(sock), network::on_receive_f(0), bind(&tcp_sender::on_disconnected, this, "disconnected"), trace_error);
    }

private:
    tunnel_point    point_;

    optional<network::tcp_socket>       sock_;
    optional<network::async_connector>  connector_;
    async_timer                         reconnect_timer_;
    posix_time::time_duration           reconnect_timeout_;
};



