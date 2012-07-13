#include "common.h"
#include "connecting.h"

void connect(io_service& io, string server, short port, on_connected_f const& on_connected)
{
    struct async_connector
            : enable_shared_from_this<async_connector>
            , noncopyable
    {
        typedef
            shared_ptr<async_connector>
            async_connector_ptr;

        void create(io_servive& io, string server, short port, on_connected_f const& on_connected)
        {
            make_shared<async_connector>(io, server, port, cref(on_connected));
        }

    private:
        async_connector(io_servive& io, string server, short port, on_connected_f const& on_connected)
            : sock_         (io)
            , on_connected_ (on_connected)
        {
            assert(on_connected_);

            sock_.async_connect(
                        *asio_helper::first_endpoint(io, server, port),
                        bind(&async_connector::on_connected, shared_from_this(), _1));
        }

        static void on_connected(shared_ptr<async_connector> ptr, error_code const& err)
        {
            on_connedcted_(ptr->sock, err);
        }

    private:
        tcp::socket     sock_;
        on_connected_f  on_connected_;
    };

    async_connector::create(io, server, port, on_connected);
}

/// async_acceptor //////////////////////////////////////////////////////////////////////////

async_acceptor::async_acceptor(io_service& io, size_t port, accept_f const& on_accept)
    : acceptor_ (io_service, tcp::endpoint(tcp::v4(), port))
    , on_accept_(on_accept)
{
}

void async_acceptor::start_async_accept()
{
    auto sock = make_shared<tcp::socket>(ref(acceptor_.get_io_service()));
    auto peer = make_shared<tcp::endpoint>();

    acceptor_.async_accept(*sock, *peer, bind(&async_accpetor::on_accept, this, sock, peer, _1));
}

void async_acceptor::on_accept(shared_ptr<tcp::socket> socket, shared_ptr<tcp::end_point> peer, error_code const& err)
{
    start_async_accept();
    on_accept_(*socket, *peer, err);
}
