#pragma once

#include "connecting.h"
#include "asio_helper.h"

struct tcp_socket
{
    typedef function<void(const void*, size_t)> on_receive_f;

    tcp_socket(tcp::socket& moveable_sock, on_receive_f const& on_receive)
        : sock_         (move(moveable_sock))
        , ready_send_   (true)
        , on_receive_   (on_receive)
    {
        sock_.async_read_some(buffer(buf_), bind(&tcp_socket::on_receive, this, _1, _2));
    }

    void send(const void* data, size_t size)
    {
        asio_helper::shared_const_buffer buf(
            static_cast<const char*>(data),
            static_cast<const char*>(data) + size);

        if (msgs_.empty() && ready_send_)
        {
            async_write(sock_, buf, bind(&tcp_socket::on_send, this, _1, _2));
            ready_send_ = false;
        }
        else
            msgs_.push_back(buf);
    }

private:
    void on_send(const error_code& error,  size_t)
    {
        if (error)
        {
            cout << error.message() << endl;
            // todo
            return;
        }

        if (!msgs_.empty())
        {
            buf_seq_ = buf_seq_t(forward<msg_list_t>(msgs_));
            msgs_ = msg_list_t();

            async_write(sock_, buf_seq_, bind(&tcp_socket::on_send, this, _1, _2));
        }
        else
            ready_send_ = true;
    }

private:
    void on_receive(const error_code& error, size_t bytes_transferred)
    {
        if (error)
        {
            cout << error.message() << endl;
            // todo
            return;
        }

        on_receive_(&buf_[0], bytes_transferred);
        sock_.async_read_some(buffer(buf_), bind(&tcp_socket::on_receive, this, _1, _2));
    }

private:
    tcp::socket sock_;

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
};


struct tcp_fragment_socket
{
    typedef tcp_socket::on_receive_f on_receive_f;

    tcp_fragment_socket(tcp::socket& moveable_sock, on_receive_f const& on_receive)
        : sock_         (moveable_sock, bind(&tcp_fragment_socket::on_receive, this, _1, _2))
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
