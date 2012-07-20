#pragma once
#include "common.h"

namespace network
{

template<class protocol>
struct transfer_strategy;

template<>
struct transfer_strategy<tcp>
{
    template<class buffer, class callback>
    static void async_send(tcp::socket& sock, buffer const& buf, callback const& cb)
    {
        async_write(sock, buf, cb);
    }

    template<class buffer, class callback>
    static void async_recv(tcp::socket& sock, buffer const& buf, callback const& cb)
    {
        sock.async_read_some(buf, cb);
    }
};

template<>
struct transfer_strategy<udp>
{
    template<class buffer, class callback>
    static void async_send(udp::socket& sock, buffer const& buf, callback const& cb)
    {
        sock.async_send(buf, cb);
    }

    template<class buffer, class callback>
    static void async_recv(udp::socket& sock, buffer const& buf, callback const& cb)
    {
        sock.async_receive(buf, cb);
    }
};

} // namespace network
