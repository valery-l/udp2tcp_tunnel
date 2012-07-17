#pragma once

#include "connecting.h"
#include "asio_helper.h"

struct tcp_socket
{
    typedef function<void(const void*, size_t)>     on_receive_f     ;
    typedef function<void()                   >     on_disconnected_f;
    typedef function<void(error_code const&)  >     on_error_t       ;

    tcp_socket(
        tcp::socket& moveable_sock,
        on_receive_f        const& on_receive,
        on_disconnected_f   const& on_discon,
        on_error_t          const& on_error)

        : transport_    (new transport(move(moveable_sock)))
        , ready_send_   (true)

        , on_receive_   (on_receive)
        , on_discon_    (on_discon )
        , on_error_     (on_error  )
    {
        // need to assign for receiving data, even socket is only for sending - it allows to receive disconnect notification "boost::asio::error::misc_errors::eof"
        sock().async_read_some(buffer(buf_), bind(&tcp_socket::on_receive, this, transport_, _1, _2));
    }

    ~tcp_socket()
    {
        transport_->alive = false;
        transport_->sock.shutdown(tcp::socket::shutdown_both);
    }

    void send(const void* data, size_t size)
    {
        asio_helper::shared_const_buffer buf(
            static_cast<const char*>(data),
            static_cast<const char*>(data) + size);

        if (msgs_.empty() && ready_send_)
        {
            async_write(
                sock(),
                buf,
                bind(&tcp_socket::on_send, this, transport_, _1, _2));

            ready_send_ = false;
        }
        else
            msgs_.push_back(buf);
    }

// tcp transport
private:
    struct transport
    {
        transport(tcp::socket&& sock)
            : alive(true)
            , sock (forward<tcp::socket>(sock))
        {
        }

        bool        alive;
        tcp::socket sock ;
    };

    typedef shared_ptr<transport> transport_ptr;

private:
    tcp::socket& sock()
    {
        return transport_->sock;
    }

private:
    void on_send(transport_ptr transport, const error_code& code,  size_t)
    {
        if (!transport->alive)
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

            async_write(
                sock(),
                buf_seq_,
                bind(&tcp_socket::on_send, this, transport_, _1, _2));
        }
        else
            ready_send_ = true;
    }

private:
    void on_receive(transport_ptr transport, const error_code& code, size_t bytes_transferred)
    {
        if (!transport->alive)
            return;

        if (code)
        {
            if (code.category() == error::misc_category &&
                code.value   () == error::eof)

                on_discon_();
            else
                on_error_(code);

            return;
        }

        on_receive_(&buf_[0], bytes_transferred);
        sock().async_read_some(buffer(buf_), bind(&tcp_socket::on_receive, this, transport_, _1, _2));
    }

private:
    transport_ptr transport_;

// for sending
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

// other notifications
private:
    on_disconnected_f   on_discon_;
    on_error_t          on_error_;
};

struct tcp_fragment_socket
{
    typedef tcp_socket::on_receive_f        on_receive_f;
    typedef tcp_socket::on_disconnected_f   on_disconnected_f;
    typedef tcp_socket::on_error_t          on_error_t;

    tcp_fragment_socket(
        tcp::socket& moveable_sock,
        on_receive_f        const& on_receive,
        on_disconnected_f   const& on_discon,
        on_error_t          const& on_error)

        : sock_         (moveable_sock, bind(&tcp_fragment_socket::on_receive, this, _1, _2), on_discon, on_error)
        , on_receive_   (on_receive)

    {
        const size_t reserve_msg_size = 1 << 16;
        partial_msg_.reserve(reserve_msg_size);
    }

    void send(const void* data, size_t size)
    {
        size_hdr_t sz(size);

        sock_.send(&sz , sizeof(sz));
        sock_.send(data, size);
    }

private:
    typedef int32_t size_hdr_t;

private:
    void on_receive(const void* data, size_t size)
    {
        while (size > 0)
        {
            bool reading_size = partial_msg_.size() < sizeof(size_hdr_t);

            if (reading_size)
                reading_size = !fill_buffer(sizeof(size_hdr_t) - partial_msg_.size(), data, size);

            if (!reading_size)
            {
                size_hdr_t sz = *reinterpret_cast<const size_hdr_t*>(&partial_msg_.front());

                if(fill_buffer(sizeof(size_hdr_t) + sz - partial_msg_.size(), data, size))
                {
                    on_receive_(&partial_msg_.front() + sizeof(size_hdr_t), sz);
                    partial_msg_.clear();
                }
            }
        }
    }

private:
    bool fill_buffer(size_t to_fill, const void*& data, size_t& received)
    {
        size_t offset = std::min(to_fill, received);

        const char* char_data = reinterpret_cast<const char*>(data);
        partial_msg_.insert(partial_msg_.end(), char_data, char_data + offset);

        char_data   += offset;
        received    -= offset;

        data = char_data;

        return to_fill == offset;
    }


private:
    tcp_socket sock_;

// for fragment receiving
private:
    on_receive_f    on_receive_;
    vector<char>    partial_msg_;
};
