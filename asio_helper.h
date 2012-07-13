#pragma once
#include "common.h"

namespace asio_helper
{

// todo: make async
template <class Protocol>
typename Protocol::resolver::iterator first_endpoint(io_service& io, string server, short port)
{
    typedef typename Protocol::resolver  resolver_t;

    resolver_t res(io);
    typename resolver_t::query q(server, lexical_cast<string>(port));

    return res.resolve(q);
}

class shared_const_buffer
{
public:
    template<class byte_it>
    shared_const_buffer(byte_it begin, byte_it end)

        : data_  (make_shared<array_t>(begin, end))
        , buffer_(buffer(*data_))
    {
    }

    // Implement the ConstBufferSequence requirements.
    typedef const_buffer value_type;
    typedef const const_buffer* const_iterator;

    const_iterator begin() const { return &buffer_    ; }
    const_iterator end  () const { return &buffer_ + 1; }

    operator const_buffer() const{ return buffer_; }

private:
    typedef vector<char> array_t;

private:
    shared_ptr<array_t> data_;
    const_buffer        buffer_;
};

// Implement the ConvertibleToConstBuffer requirements for shared_const_buffer
inline const void* buffer_cast_helper(const shared_const_buffer& b) { return buffer_cast<const void*>(*(b.begin())); }
inline std::size_t buffer_size_helper(const shared_const_buffer& b) { return buffer_size(*(b.begin()))             ; }

template<class const_buffer_type>
class shared_const_buffers_seq
{
public:
    typedef list<const_buffer_type> buffer_seq_t;

public:
    shared_const_buffers_seq()
    {
    }

    template<class buffer_it>
    shared_const_buffers_seq(buffer_it begin, buffer_it end)
        : data_(make_shared<buffer_seq_t>(begin, end))
    {
    }

    explicit shared_const_buffers_seq(buffer_seq_t&& seq)
        : data_(make_shared<buffer_seq_t>(std::forward<buffer_seq_t>(seq)))
    {
    }

    // Implement the ConstBufferSequence requirements.
    typedef typename buffer_seq_t::value_type      value_type;
    typedef typename buffer_seq_t::const_iterator  const_iterator;

    const_iterator begin() const { return data_->begin(); }
    const_iterator end  () const { return data_->end  (); }

private:
    shared_ptr<buffer_seq_t> data_;
};


} // namespace details
