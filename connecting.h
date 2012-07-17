#pragma once
#include "common.h"
#include "asio_helper.h"

namespace network
{

typedef function<void (tcp::socket&)>                           on_connected_f; // moveable socket
typedef function<void (tcp::socket&, tcp::endpoint const&)>     on_accept_f   ; // moveable socket
typedef function<void (error_code const&)>                      on_error_f    ;

void connect(io_service& io, string server, short port, on_connected_f const&, on_error_f const&);

struct async_acceptor
        : noncopyable
{
    async_acceptor(io_service& io, size_t port, on_accept_f const& on_accept, on_error_f const& on_error);
   ~async_acceptor();

private:
    shared_ptr<struct underlying_acceptor> acceptor_;
};

} // namespace network
