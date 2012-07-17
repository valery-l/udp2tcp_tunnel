#pragma once
#include "common.h"
#include "asio_helper.h"

typedef
    function<void (tcp::socket&, error_code const&)> // moveable socket
    on_connected_f;

void connect(io_service& io, string server, short port, on_connected_f const& on_connected);

struct async_acceptor
        : noncopyable
        , enable_shared_from_this<async_acceptor>
{
    typedef
        shared_ptr<async_acceptor>
        acceptor_ptr;

    typedef
        function<void (tcp::socket&, tcp::endpoint const&, error_code const&)> // moveable socket
        on_accept_f;

public:
    static acceptor_ptr create(io_service& io, size_t port, on_accept_f const& on_accept);

private:
    async_acceptor(io_service& io, size_t port, on_accept_f const& on_accept);

    void start_async_accept ();
    void on_accept          (acceptor_ptr that, shared_ptr<tcp::socket> socket, shared_ptr<tcp::endpoint> peer, error_code const& err);


private:
    tcp::acceptor   acceptor_;
    on_accept_f     on_accept_;
};

