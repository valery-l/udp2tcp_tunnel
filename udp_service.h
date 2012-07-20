#pragma once
#include "net_common.h"

namespace network
{

struct udp_socket
{
    udp_socket(
        io_service& io,
        optional<endpoint> const& local_bind,
        optional<endpoint> const& remote_server,
        on_receive_f const&,
        on_error_f const&);

   ~udp_socket();

    void send(const void* data, size_t size);

private:
    shared_ptr<underlying_transport> transport_;
};

}
