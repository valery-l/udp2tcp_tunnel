#include "common.h"
#include "tcp_service.h"
#include "underlying_transport_impl.h"

namespace network
{

//////////////////////////////////////////////////////////////////////
// tcp_socket
tcp_socket::tcp_socket(
    tcp::socket&               moveable_sock,
    on_receive_f        const& on_receive,
    on_disconnected_f   const& on_discon,
    on_error_f          const& on_error)

    : on_discon_(on_discon )
    , on_error_ (on_error  )
    , transport_(underlying_transport_impl<tcp>::create(move(moveable_sock), on_receive, bind(&tcp_socket::on_error, this, _1)))
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

void tcp_socket::on_error(error_code const& code)
{
    if (code)
    {
        if (code.category() == error::misc_category &&
            code.value   () == error::eof)
        {
            on_discon_();
        }
        else
            on_error_(code);
    }
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
