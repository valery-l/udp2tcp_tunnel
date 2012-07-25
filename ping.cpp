// From Boost samples (with some modifications)
// link: http://www.boost.org/doc/libs/1_48_0/doc/html/boost_asio/example/icmp/icmp_header.hpp


// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "common.h"
#include "ping.h"

#include "asio_helper.h"

#include "icmp_header.h"
#include "ipv4_header.h"
#include "auto_cancel.h"

namespace network
{

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;

struct async_pinger::impl
        : enable_shared_from_this<impl>
        , enable_cancel_resource
{
    typedef
        shared_ptr<impl>
        ptr_t;

    static ptr_t create(io_service& io, string destination, posix_time::milliseconds const& timeout, network::on_pinged_f const& on_pinged)
    {
        ptr_t pinger(new impl(io, destination, timeout, on_pinged));
        pinger->start_ping();

        return pinger;
    }

private:
    impl(io_service& io, string destination, posix_time::milliseconds const& timeout, network::on_pinged_f const& on_pinged)

        : enable_cancel_resource(socket_)

        , socket_   (io, icmp::v4())
        , timer_    (io)
        , timeout_  (timeout)
        , on_pinged_(on_pinged)
    {
        icmp::resolver::query query(icmp::v4(), destination, "");
        destination_ = *icmp::resolver(io).resolve(query);
    }

    void start_ping()
    {
        send_request ();
        start_receive();
    }

private:
    void on_send(size_t bytes_transferred)
    {
        assert(bytes_transferred == sizeof(icmp_header));
    }

    void send_request()
    {
        time_sent_ = posix_time::microsec_clock::universal_time();

        // Create an ICMP header for an echo request.
        icmp_header echo_request;

        echo_request.type(icmp_header::echo_request);
        echo_request.code(0);
        echo_request.identifier(get_identifier());
        echo_request.unique_number((unsigned short)(time_sent_ - epoch_start_).total_milliseconds());
        compute_checksum<const char*>(echo_request, 0, 0);

        asio_helper::shared_const_buffer shared_buf(&echo_request, sizeof(echo_request));
        socket_.async_send_to(shared_buf, destination_, bind(&impl::on_send, shared_from_this(), _2));

        // Wait up to timeout seconds for a reply.
        timer_.expires_at(time_sent_ + timeout_);
        timer_.async_wait(boost::bind(&impl::on_timeout, shared_from_this(), _1));
    }

    void on_timeout(error_code const& code)
    {
        if (!code)
        {
            on_pinged_(false, timeout_);
            socket_.cancel();
        }
    }

    void start_receive()
    {
        // Discard any data already in the buffer.
        reply_buffer_.consume(reply_buffer_.size());

        // Wait for a reply. We prepare the buffer to receive up to 64KB.
        socket_.async_receive(reply_buffer_.prepare(1 << 16),
                              boost::bind(&impl::on_receive, shared_from_this(), _1, _2));
    }

    void on_receive(error_code const& code, std::size_t length)
    {
        if (cancelled())
            return;

        if (code)
            return;

        reply_buffer_.commit(length);

        std::istream is(&reply_buffer_);
        ipv4_header ipv4_hdr;
        icmp_header icmp_hdr;

        is >> ipv4_hdr >> icmp_hdr;

        // We can receive all ICMP packets received by the host, so we need to
        // filter out only the echo replies that match the our identifier and
        // expected sequence number.
        if (is                                                          &&
            ipv4_hdr.source_address() == destination_.address().to_v4() &&
            icmp_hdr.type() == icmp_header::echo_reply                  &&
            icmp_hdr.identifier() == get_identifier()                   &&
            icmp_hdr.unique_number() == (unsigned short)(time_sent_ - epoch_start_).total_milliseconds())
        {
            timer_.cancel();

            posix_time::ptime now = posix_time::microsec_clock::universal_time();
            on_pinged_(true, posix_time::milliseconds((now - time_sent_).total_milliseconds()));
        }
        else
            start_receive();
    }

    static unsigned short get_identifier()
    {
#if defined(BOOST_WINDOWS)
        return static_cast<unsigned short>(::GetCurrentProcessId());
#else
        return static_cast<unsigned short>(::getpid());
#endif
    }

private:
    icmp::endpoint  destination_;
    icmp::socket    socket_;
    deadline_timer  timer_;

    posix_time::ptime       time_sent_;
    boost::asio::streambuf  reply_buffer_;

    posix_time::milliseconds    timeout_;
    network::on_pinged_f        on_pinged_;

    size_t unique_number_;

private:
    static posix_time::ptime epoch_start_;
};

///////////////////////////////////////
posix_time::ptime async_pinger::impl::epoch_start_(boost::gregorian::date(1970, 1, 1));

} // 'anonymous'






///////////////////////////////////////////////////////////////////////////////////////
network::async_pinger::async_pinger(io_service& io, string destination, posix_time::milliseconds const& timeout, on_pinged_f const& on_pinged)
    : pimpl_(impl::ptr_t(impl::create(io, destination, timeout, on_pinged)))
{
}

network::async_pinger::~async_pinger()
{
}


