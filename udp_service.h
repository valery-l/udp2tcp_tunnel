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

    void send(const void* data, size_t size, optional<endpoint> const& destination = none);

private:
    struct impl;
    shared_ptr<impl> pimpl_;
};

}
