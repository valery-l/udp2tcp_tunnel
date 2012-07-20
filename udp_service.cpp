#include "common.h"

#include "asio_helper.h"
#include "net_common.h"
#include "udp_service.h"
#include "underlying_transport_impl.h"

namespace
{
udp::socket connected_udp_socket(
    io_service& io,
    optional<network::endpoint> const& local_bind,
    optional<network::endpoint> const& remote_server)
{
    udp::socket sock = local_bind ? udp::socket(io, *local_bind) : udp::socket(io);

//    if (local_bind)
//        sock.bind(*local_bind);

    if (remote_server)
        sock.connect(*remote_server);

    sock.set_option(udp::socket::broadcast(true));
    return sock;
}

} // 'anonymous'


namespace network
{

udp_socket::udp_socket(
    io_service& io,
    optional<endpoint> const& local_bind,
    optional<endpoint> const& remote_server,
    on_receive_f const& on_receive,
    on_error_f   const& on_error)

    : transport_(
        underlying_transport_impl<udp>::create(
            move(
                connected_udp_socket(
                    io,
                    local_bind,
                    remote_server)),
            on_receive,
            on_error))
{
}

udp_socket::~udp_socket()
{
    transport_->close_connection();
}

void udp_socket::send(const void* data, size_t size)
{
    transport_->send(data, size);
}

} // namespace network
