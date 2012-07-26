#include "common.h"
#include "tunnel_udp_service.h"

using namespace network;

namespace
{

vector<char> unite_with_endpoint(const void* data, size_t size, endpoint const& point)
{
    vector<char> buf;
    buf.reserve(size + endpoint::serialize_size());

    point.serialize(back_inserter(buf));

    char const* bytes = reinterpret_cast<char const*>(data);
    copy(bytes, bytes + size, back_inserter(buf));

    return buf;
}

} // 'anonymous'

//////////////////////////////////////////////////////////////
udp_sender::udp_sender(io_service& io)
    : sock_ (io, none, none, on_receive_f(0), trace_error)
{
}

void udp_sender::send(const void* data, size_t size)
{
    size_t hdr_size = endpoint::serialize_size();
    assert(size >= hdr_size);

    const char* bytes = reinterpret_cast<const char*>(data);
    sock_.send(bytes + hdr_size, size - hdr_size, endpoint(bytes));
}


//////////////////////////////////////////////////////////////
udp_receiver::udp_receiver(io_service& io, endpoint const& local_bind, endpoint const& remote_server, tcp_sender_ptr sndr, string remote_tcp_point)
    : sock_             (io, local_bind, none, bind(&udp_receiver::on_receive, this, _1, _2), trace_error)
    , sender_           (sndr)
    , remote_server_    (remote_server)
    , remote_tcp_point_ (remote_tcp_point)
{
}

void udp_receiver::on_receive(const void* data, size_t size)
{
    vector<char> buf = unite_with_endpoint(data, size, remote_server_);
    sender_->send(&buf[0], buf.size(), remote_tcp_point_);
}
