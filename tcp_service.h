#pragma once

#include "connecting.h"
#include "asio_helper.h"
#include "connecting.h"
#include "net_common.h"

namespace network
{

typedef function<void ()>   on_disconnected_f;

//////////////////////////////////////////////////
// socket for sending and receiving streaming data
struct tcp_socket
    : noncopyable
{
    tcp_socket(tcp::socket& moveable_sock, on_receive_f const&, on_disconnected_f const&, on_error_f const&);
   ~tcp_socket();

    void send(const void* data, size_t size);

private:
    void on_error(error_code const& code);

// tcp transport
private:
    on_disconnected_f   on_discon_;
    on_error_f          on_error_;

private:
    shared_ptr<underlying_transport> transport_;
};


////////////////////////////////////////////////////////////////////////////////
// wrapper for fragmented super-protocol (to break tcp data stream into messages)
struct tcp_fragment_wrapper
    : noncopyable
{
    tcp_fragment_wrapper(tcp::socket& moveable_sock, on_receive_f const&, on_disconnected_f const&, on_error_f const&);

    void send(const void* data, size_t size);

private:
    typedef int32_t size_hdr_t;

private:
    void on_receive(const void* data, size_t size);
    bool fill_buffer(size_t to_fill, const void*& data, size_t& received);

private:
    tcp_socket sock_;

// for fragment receiving
private:
    on_receive_f    on_receive_;
    vector<char>    partial_msg_;
};

} // namespace network
