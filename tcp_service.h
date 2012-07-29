#pragma once
#include "net_common.h"

namespace network
{

typedef function<void (error_code const&)>   on_disconnected_f;

//////////////////////////////////////////////////
// socket for sending and receiving streaming data
struct tcp_socket
    : noncopyable
{
    // can't use r-value ref, as it's not supported by boost::in_place and has problems with boost::function
    tcp_socket(tcp::socket& moveable_sock, on_receive_f const&, on_disconnected_f const&, on_error_f const&);

    void send(const void* data, size_t size);

private:
    struct impl;
    shared_ptr<impl> pimpl_;
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
