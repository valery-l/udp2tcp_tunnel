#include "common.h"
#include "tunnel.h"

using namespace network;


void tunnel::create_udp_receiver(network::endpoint const& local_bind, network::endpoint const& remote_server, string remote_tcp_point)
{
    if (sender_ == nullptr)
    {
        cerr << local_bind << "udp listening point creating was skipped as no tcp client/server was mentioned before" << endl;
        return;
    }

    receivers_[local_bind] = make_shared<udp_receiver>(ref(io_), local_bind, remote_server, sender_, remote_tcp_point);
}

void tunnel::create_tcp_sender(bool client, network::endpoint const& point, posix_time::time_duration const& reconnect_timeout)
{
    sender_ = client
        ? tcp_sender_ptr(make_shared<tcp_client>(ref(io_), point, reconnect_timeout))
        : tcp_sender_ptr(make_shared<tcp_server>(ref(io_), point));
}

void tunnel::run()
{
    io_.run();
}

