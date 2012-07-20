#pragma once
#include "net_common.h"
#include "asio_helper.h"
#include "transfer_strategy.h"

namespace network
{

////////////////////////////////////////////////////////////////////////////////////
// underlying_transport for socket
template <class protocol, class async_transfer_strategy = transfer_strategy<protocol> >
struct underlying_transport_impl
        : noncopyable
        , underlying_transport
        , enable_shared_from_this
            <underlying_transport_impl
                <protocol, async_transfer_strategy> >
{
    typedef underlying_transport_impl   this_t;
    typedef shared_ptr<this_t>          ptr_t;
    typedef async_transfer_strategy     strat_t;
    typedef typename protocol::socket   socket_t;

    static ptr_t create(socket_t&& sock, on_receive_f const& on_receive, on_error_f const& on_error)
    {
        ptr_t ptr(new this_t(forward<socket_t>(sock), on_receive, on_error));
        strat_t::async_recv(ptr->sock_, buffer(ptr->buf_), bind(&this_t::on_receive, ptr, _1, _2));

        return ptr;
    }

    void close_connection()
    {
        alive_ = false;
        sock_.shutdown(socket_t::shutdown_both);
    }

    void send(const void* data, size_t size)
    {
        asio_helper::shared_const_buffer buf(
            static_cast<const char*>(data),
            static_cast<const char*>(data) + size);

        if (msgs_.empty() && ready_send_)
        {
            strat_t::async_send(
                sock_,
                buf,
                bind(&this_t::on_send, this->shared_from_this(), _1, _2));

            ready_send_ = false;
        }
        else
            msgs_.push_back(buf);
    }

private:
    underlying_transport_impl(socket_t&& sock, on_receive_f const& on_receive, on_error_f const& on_error)
        : alive_            (true)
        , sock_             (forward<socket_t>(sock))
        , ready_send_       (true)
        , on_receive_       (on_receive)
        , on_error_         (on_error)
    {
    }

private:
    void on_send(const error_code& code,  size_t)
    {
        if (!alive_)
            return;

        if (code)
        {
            on_error_(code);
            return;
        }

        if (!msgs_.empty())
        {
            buf_seq_ = buf_seq_t(forward<msg_list_t>(msgs_));
            msgs_    = msg_list_t();

            strat_t::async_send(
                sock_,
                buf_seq_,
                bind(&this_t::on_send, this->shared_from_this(), _1, _2));
        }
        else
            ready_send_ = true;
    }

private:
    void on_receive(const error_code& code, size_t bytes_transferred)
    {
        if (!alive_)
            return;

        if (code)
        {
            if (on_error_)
                on_error_(code);

            return;
        }

        if (on_receive_)
            on_receive_(&buf_[0], bytes_transferred);

        strat_t::async_recv(sock_, buffer(buf_), bind(&this_t::on_receive, this->shared_from_this(), _1, _2));
    }

private:
    bool        alive_;
    socket_t    sock_ ;

private:
    typedef
        asio_helper::shared_const_buffers_seq<asio_helper::shared_const_buffer>
        buf_seq_t;

    typedef
        list<asio_helper::shared_const_buffer>
        msg_list_t;

private:
    bool        ready_send_;
    msg_list_t  msgs_;
    buf_seq_t   buf_seq_;


    // for receiving
private:
    static const size_t         buf_size_ = 1 << 16;

    on_receive_f                on_receive_;
    std::array<char, buf_size_> buf_;

private:
    on_error_f                  on_error_;
};

} // namespace network

