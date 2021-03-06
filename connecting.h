#pragma once
#include "common.h"
#include "net_common.h"
#include "auto_cancel.h"

namespace network
{

// can't use r-value ref on sock here, as it's not supported by boost::in_place and has problems with boost::function
typedef function<void (tcp::socket&, network::endpoint const&)> on_connected_f  ; // moveable socket
typedef function<void (error_code const&)>                      on_refused_f;
typedef function<void (tcp::socket&, network::endpoint const&)> on_accept_f     ; // moveable socket

struct async_connector
{
    async_connector(io_service& io, endpoint const& remote_server, on_connected_f const&, on_refused_f const&, on_error_f const&);
   ~async_connector();

private:
    struct impl;
    auto_cancel_ptr<impl> pimpl_;
};

struct async_acceptor
        : noncopyable
{
    async_acceptor(io_service& io, endpoint const& local_bind, on_accept_f const& on_accept, on_error_f const& on_error);
   ~async_acceptor();


private:
    struct impl;
    auto_cancel_ptr<impl> acceptor_;
};

} // namespace network
