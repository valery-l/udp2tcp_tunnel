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

    static ptr_t create(socket_t&& sock, on_receive_f const& on_receive, on_error_f const& on_error, strat_t const& strat = strat_t())
    {
        const size_t buf_size = 256 * 1024;
        sock.set_option(typename protocol::socket::send_buffer_size   (buf_size));
        sock.set_option(typename protocol::socket::receive_buffer_size(buf_size));

        ptr_t ptr(new this_t(forward<socket_t>(sock), on_receive, on_error, strat));
        strat.async_recv(ptr->sock_, buffer(ptr->buf_), bind(&this_t::on_receive, ptr, _1, _2));

        return ptr;
    }

    void close_connection()
    {
        alive_ = false;
        sock_.cancel();
    }

    void send_impl(const void *data, size_t size, udp*)
    {
        asio_helper::shared_const_buffer buf(
                    static_cast<const char*>(data),
                    static_cast<const char*>(data) + size);

        strat_.async_send(
            sock_,
            buf,
            bind(&this_t::on_send, this->shared_from_this(), _1, _2));
    }

    void send_impl(const void *data, size_t size, tcp*)
    {
        asio_helper::shared_const_buffer buf(
                    static_cast<const char*>(data),
                    static_cast<const char*>(data) + size);

        if (msgs_.empty() && ready_send_)
        {
            strat_.async_send(
                        sock_,
                        buf,
                        bind(&this_t::on_send, this->shared_from_this(), _1, _2));

            ready_send_ = false;
        }
        else
            msgs_.push_back(buf);
    }


    void send(const void* data, size_t size)
    {
        send_impl(data, size, (protocol*)0);
    }

private:
    underlying_transport_impl(socket_t&& sock, on_receive_f const& on_receive, on_error_f const& on_error, strat_t const& strat)
        : alive_            (true)
        , sock_             (forward<socket_t>(sock))
        , strat_            (strat)
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
            cout << "Pack of " << msgs_.size() << " message(s)" << endl;

            buf_seq_ = buf_seq_t(forward<msg_list_t>(msgs_));
            msgs_    = msg_list_t();

            strat_.async_send(
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

        //cout << "bytes_transferred: " << bytes_transferred << endl;

        if (on_receive_)
            on_receive_(&buf_[0], bytes_transferred);

        strat_.async_recv(sock_, buffer(buf_), bind(&this_t::on_receive, this->shared_from_this(), _1, _2));
    }

private:
    bool        alive_;
    socket_t    sock_ ;

private:
    strat_t const& strat_;

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
    static const size_t         buf_size_ = 1 << 18;

    on_receive_f                on_receive_;
    std::array<char, buf_size_> buf_;

private:
    on_error_f                  on_error_;
};

} // namespace network

