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
    transfer_strategy(udp::endpoint const& remote_peer)
        : remote_peer_(remote_peer)
    {
    }

    template<class buffer, class callback>
    void async_send(udp::socket& sock, buffer const& buf, callback const& cb) const
    {
        sock.async_send_to(buf, remote_peer_, cb);
    }

    template<class buffer, class callback>
    void async_recv(udp::socket& sock, buffer const& buf, callback const& cb) const
    {
        sock.async_receive(buf, cb);
    }

private:
    udp::endpoint remote_peer_;
};

} // namespace network
