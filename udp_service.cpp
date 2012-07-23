#include "common.h"

#include "asio_helper.h"
#include "net_common.h"
#include "udp_service.h"
#include "underlying_transport_impl.h"

namespace
{
udp::socket connected_udp_socket(
    io_service& io,
    optional<network::endpoint> const& local_bind)
{
    udp::socket sock = udp::socket(io, local_bind ? udp::endpoint(*local_bind) : udp::endpoint(ip::udp::v4(), 0));

    error_code code;

    udp::socket::receive_buffer_size rbs;
    sock.get_option(rbs, code);

    cout << "Recv buffer. ";
    if (code) cout << "Error: " << code.message();
    else      cout << rbs.value();
    cout << endl;

    cout << "Send buffer. ";
    udp::socket::send_buffer_size sbs;
    sock.get_option(sbs);

    if (code) cout << "Error: " << code.message();
    else      cout << sbs.value();
    cout << endl;

    sock.set_option(udp::socket::reuse_address(true));
    sock.set_option(udp::socket::broadcast    (true));
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

    :  strat_    (remote_server ? *remote_server : endpoint())
    ,  transport_(
        underlying_transport_impl<udp>::create(
            move(connected_udp_socket(io, local_bind)),
            on_receive,
            on_error,
            strat_))
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
