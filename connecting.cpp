#include "common.h"
#include "connecting.h"
#include "asio_helper.h"
#include "auto_cancel.h"

namespace network
{

struct async_connector::impl
        : noncopyable
        , enable_cancel_resource
{
    typedef shared_ptr<impl> ptr_t;

    static ptr_t create(io_service& io, endpoint const& remote_server, on_connected_f const& on_connected, on_error_f const& on_error)
    {
        // don't worry - the object will be deleted automatically after establishing connection
        ptr_t connector(new impl(io, on_connected, on_error));
        connector->sock_.async_connect(remote_server, bind(&impl::on_connected, connector, _1));

        return connector;
    }

private:
    impl(io_service& io, on_connected_f const& on_connected, on_error_f const& on_error)

        : enable_cancel_resource(sock_)

        , sock_         (io)
        , on_connected_ (on_connected)
        , on_error_     (on_error )
    {
        assert(on_connected_);
    }

    void on_connected(error_code const& code)
    {
        if (cancelled())
            return;

        if (code)
        {
            on_error_(code);
            return;
        }

        on_connected_(sock_);
        detach_resource();
    }


private:
    tcp::socket     sock_;
    on_connected_f  on_connected_;
    on_error_f      on_error_;
};

////////////////////////////////////////////////////////////////////////
async_connector::async_connector(io_service& io, endpoint const& remote_server, on_connected_f const& on_connected, on_error_f const& on_error)
    : pimpl_(impl::create(io, remote_server, on_connected, on_error))
{
}

async_connector::~async_connector()
{
}

///////////////////////////////////////////////////////////////////////
// underlying_acceptor

struct async_acceptor::impl
        : noncopyable
        , enable_shared_from_this<impl>
        , enable_cancel_resource
{
    typedef shared_ptr<impl>  ptr_t;

public:
    static ptr_t create(io_service& io, endpoint const& local_bind, network::on_accept_f const& on_accept, network::on_error_f const& on_error)
    {
        ptr_t acceptor(new impl(ref(io), local_bind, boost::cref(on_accept), boost::cref(on_error)));
        acceptor->start_async_accept();

        return acceptor;
    }

private:
    impl(io_service& io, endpoint const& local_bind, network::on_accept_f const& on_accept, network::on_error_f const& on_error)

        : enable_cancel_resource(acceptor_)

        , acceptor_ (io, local_bind)
        , on_accept_(on_accept)
        , on_error_ (on_error )
    {
        acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    }

    void start_async_accept()
    {
        auto sock = make_shared<tcp::socket>(ref(acceptor_.get_io_service()));
        auto peer = make_shared<tcp::endpoint>();

        acceptor_.async_accept(*sock, *peer, bind(&impl::on_accept, shared_from_this(), sock, peer, _1));
    }

    void on_accept(shared_ptr<tcp::socket> socket, shared_ptr<tcp::endpoint> peer, error_code const& code)
    {
        if (cancelled())
            return;

        if (code)
        {
            on_error_(code);
            return;
        }

        start_async_accept();
        on_accept_(*socket, *peer);
    }

private:
    tcp::acceptor           acceptor_;
    network::on_accept_f    on_accept_;
    network::on_error_f     on_error_;
};


////////////////////////////////////////////////////////////////////////
// async_acceptor
async_acceptor::async_acceptor(io_service& io, endpoint const& local_bind, on_accept_f const& on_accept, on_error_f const& on_error)
    : acceptor_(impl::create(io, local_bind, on_accept, on_error))
{
}

async_acceptor::~async_acceptor()
{
}

} // namespace network

