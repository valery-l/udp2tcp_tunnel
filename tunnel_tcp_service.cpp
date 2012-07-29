#include "common.h"
#include "tunnel_tcp_service.h"

using namespace network;

tcp_client::tcp_client(io_service& io, endpoint const& remote_server, posix_time::time_duration const& reconnect_timeout)
    : udp_sender_       (io)
    , reconnect_timer_  (io, bind(&tcp_client::start_connect, this, ref(io), remote_server))
    , reconnect_timeout_(reconnect_timeout)
{
    start_connect(io, remote_server);
}


void tcp_client::send(const void* data, size_t size, string)
{
    if (sock_)
        sock_->send(data, size);
}

void tcp_client::start_connect(io_service& io, endpoint const& remote_server)
{
    std::stringstream sstrm;
    sstrm << "Connection refused for " << remote_server;

    connector_.reset();
    connector_ = in_place(ref(io), remote_server, bind(&tcp_client::on_connected, this, _1, _2), bind(&tcp_client::on_disconnected, this, sstrm.str(), _1), error_tracer("client connector"));
}

void tcp_client::on_receive(const void* data, size_t size)
{
    udp_sender_.send(data, size);
}

void tcp_client::on_disconnected(string reason, error_code const& code)
{
    sock_.reset();
    cout << reason << " Error: " << code.category().name() << ":" << code.message() << endl;

    reconnect_timer_.wait(reconnect_timeout_);
}

void tcp_client::on_connected(tcp::socket& sock, endpoint const& remote_peer)
{
    cout << "Connection established with " << remote_peer << endl;

    std::stringstream sstrm;
    sstrm << "Connection lost with " << remote_peer;

    sock_ = in_place(ref(sock), bind(&tcp_client::on_receive, this, _1, _2), bind(&tcp_client::on_disconnected, this, sstrm.str(), _1), error_tracer("client socket"));
}


//////////////////////////////////////////////////////////////////////
tcp_server::tcp_server(io_service& io, endpoint const& local_bind)
    : udp_sender_(io)
    , acceptor_  (io, local_bind, bind(&tcp_server::on_accept, this, _1, _2), error_tracer("server acceptor"))
{
}

void tcp_server::send(const void* data, size_t size, string client)
{
    if (client.empty())
    {
        for (auto it = clients_.begin(); it != clients_.end(); ++it)
            it->second->send(data, size);
    }
    else
    {
        clients_t::iterator it = clients_.find(client);

        if (it != clients_.end())
            it->second->send(data, size);
    }
}

void tcp_server::on_accept(tcp::socket& sock, endpoint const& remote_peer)
{
    cout << "Connection with " << remote_peer.addr << " is established" << endl;

    clients_[remote_peer.addr.to_string()] =
        make_shared<tcp_fragment_wrapper>(
            ref (sock),
            bind(&tcp_server::on_receive     , this, _1, _2),
            bind(&tcp_server::on_disconnected, this, remote_peer.addr.to_string(), _1),
            error_tracer("tcp_client"));
}

void tcp_server::on_receive(const void* data, size_t size)
{
    udp_sender_.send(data, size);
}

void tcp_server::on_disconnected(string client, error_code const& code)
{
    clients_.erase(client);
    cout << "Connection lost with " << client << " Error: " << code.category().name() << ":" << code.message() << endl;
}
