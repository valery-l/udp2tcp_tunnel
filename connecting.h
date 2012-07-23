#pragma once
#include "common.h"
#include "net_common.h"

namespace network
{

// can't use r-value ref on sock here, as it's not supported by boost::in_place and has problems with boost::function
typedef function<void (tcp::socket&)>                       on_connected_f  ; // moveable socket
typedef function<void (tcp::socket&, tcp::endpoint const&)> on_accept_f     ; // moveable socket

void connect(io_service& io, endpoint const& remote_server, on_connected_f const&, on_error_f const&);

struct async_acceptor
        : noncopyable
{
    async_acceptor(io_service& io, endpoint const& local_bind, on_accept_f const& on_accept, on_error_f const& on_error);
   ~async_acceptor();

private:
    shared_ptr<struct underlying_acceptor> acceptor_;
};

} // namespace network
