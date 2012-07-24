#pragma once
#include "common.h"

namespace network
{

typedef function<void (const void*, size_t)>    on_receive_f;
typedef function<void (error_code const&)>      on_error_f  ;

struct endpoint
{
    endpoint(string address = "", size_t port = 0)
        : addr  (address.empty() ? ip::address_v4::any() : ip::address_v4::from_string(address))
        , port  (port)
    {}

    endpoint(ip::address_v4 address, size_t port = 0)
        : addr  (address)
        , port  (port   )
    {}

    operator tcp::endpoint() const { return tcp::endpoint(addr, port); }
    operator udp::endpoint() const { return udp::endpoint(addr, port); }

    ip::address_v4  addr;
    size_t          port;
};

} // namespace network
