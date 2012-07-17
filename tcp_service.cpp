#include "common.h"
#include "tcp_service.h"

namespace network
{

////////////////////////////////////////////////////////////////////////////////////
// underlying_transport for tcp_socket
struct underlying_transport
        : noncopyable
        , enable_shared_from_this<underlying_transport>
{
    typedef shared_ptr<underlying_transport> ptr_t;

    static ptr_t create(tcp::socket&& sock,
                      network::on_receive_f        const& on_receive,
                      network::on_disconnected_f   const& on_discon,
                      network::on_error_f          const& on_error)
    {
        ptr_t ptr(new underlying_transport(forward<tcp::socket>(sock), on_receive, on_discon, on_error));
        ptr->sock_.async_read_some(buffer(ptr->buf_), bind(&underlying_transport::on_receive, ptr, _1, _2));

        return ptr;
    }

    void close_connection()
    {
        alive_ = false;
        sock_.shutdown(tcp::socket::shutdown_both);
    }

    void send(const void* data, size_t size)
    {
        asio_helper::shared_const_buffer buf(
            static_cast<const char*>(data),
            static_cast<const char*>(data) + size);

        if (msgs_.empty() && ready_send_)
        {
            async_write(
                sock_,
                buf,
                bind(&underlying_transport::on_send, shared_from_this(), _1, _2));

            ready_send_ = false;
        }
        else
            msgs_.push_back(buf);
    }

private:
    underlying_transport(
        tcp::socket&& sock,
        network::on_receive_f        const& on_receive,
        network::on_disconnected_f   const& on_discon,
        network::on_error_f          const& on_error)

        : alive_        (true)
        , sock_         (forward<tcp::socket>(sock))
        , ready_send_   (true)
        , on_receive_   (on_receive)
        , on_discon_    (on_discon )
        , on_error_     (on_error  )
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

            async_write(
                sock_,
                buf_seq_,
                bind(&underlying_transport::on_send, shared_from_this(), _1, _2));
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
            if (code.category() == error::misc_category &&
                code.value   () == error::eof)
            {
                on_discon_();
            }
            else
                on_error_(code);

            return;
        }

        on_receive_(&buf_[0], bytes_transferred);
        sock_.async_read_some(buffer(buf_), bind(&underlying_transport::on_receive, shared_from_this(), _1, _2));
    }

private:
    bool        alive_;
    tcp::socket sock_ ;

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

    network::on_receive_f       on_receive_;
    std::array<char, buf_size_> buf_;

    // other notifications
private:
    network::on_disconnected_f   on_discon_;
    network::on_error_f          on_error_;
};

//////////////////////////////////////////////////////////////////////
// tcp_socket
tcp_socket::tcp_socket(
    tcp::socket&               moveable_sock,
    on_receive_f        const& on_receive,
    on_disconnected_f   const& on_discon,
    on_error_f          const& on_error)

    : transport_(underlying_transport::create(move(moveable_sock), on_receive, on_discon, on_error))
{
}

tcp_socket::~tcp_socket()
{
    transport_->close_connection();
}

void tcp_socket::send(const void* data, size_t size)
{
    transport_->send(data, size);
}


///////////////////////////////////////////////////////////////////////
// tcp_fragment_socket
tcp_fragment_wrapper::tcp_fragment_wrapper(
    tcp::socket&               moveable_sock,
    on_receive_f        const& on_receive,
    on_disconnected_f   const& on_discon,
    on_error_f          const& on_error)

    : sock_         (moveable_sock, bind(&tcp_fragment_wrapper::on_receive, this, _1, _2), on_discon, on_error)
    , on_receive_   (on_receive)

{
    const size_t reserve_msg_size = 1 << 16;
    partial_msg_.reserve(reserve_msg_size);
}

void tcp_fragment_wrapper::send(const void* data, size_t size)
{
    size_hdr_t sz(size);

    sock_.send(&sz , sizeof(sz));
    sock_.send(data, size);
}

void tcp_fragment_wrapper::on_receive(const void* data, size_t size)
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

bool tcp_fragment_wrapper::fill_buffer(size_t to_fill, const void*& data, size_t& received)
{
    size_t offset = std::min(to_fill, received);

    const char* char_data = reinterpret_cast<const char*>(data);
    partial_msg_.insert(partial_msg_.end(), char_data, char_data + offset);

    char_data   += offset;
    received    -= offset;

    data = char_data;

    return to_fill == offset;
}

} // namespace network
