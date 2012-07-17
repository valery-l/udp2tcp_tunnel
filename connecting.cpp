#include "common.h"
#include "connecting.h"

void connect(io_service& io, string server, short port, on_connected_f const& on_connected)
{
    struct async_connector
            : noncopyable
    {
        static void request(io_service& io, string server, short port, on_connected_f const& on_connected)
        {
            // don't worry - the object will be deleted automatically after establishing connection
            async_connector_ptr connector(new async_connector(io, on_connected));

            connector->sock_.async_connect(
                *asio_helper::first_endpoint<tcp>(io, server, port),
                bind(&async_connector::on_connected, connector, _1));

        }

    private:

        typedef
            shared_ptr<async_connector>
            async_connector_ptr;

        async_connector(io_service& io, on_connected_f const& on_connected)
            : sock_         (io)
            , on_connected_ (on_connected)
        {
            assert(on_connected_);
        }

        void on_connected(error_code const& err)
        {
            if (err)
            {
                cout << err.message() << endl;
                // todo
                return;
            }

            on_connected_(sock_, err);
        }


    private:
        tcp::socket     sock_;
        on_connected_f  on_connected_;
    };

    async_connector::request(io, server, port, on_connected);
}

/// async_acceptor //////////////////////////////////////////////////////////////////////////
auto async_acceptor::create(io_service& io, size_t port, on_accept_f const& on_accept) -> acceptor_ptr
{
    acceptor_ptr acceptor(new async_acceptor(ref(io), port, cref(on_accept)));
    acceptor->start_async_accept();

    return acceptor;
}

async_acceptor::async_acceptor(io_service& io, size_t port, on_accept_f const& on_accept)
    : acceptor_ (io, tcp::endpoint(tcp::v4(), port))
    , on_accept_(on_accept)
{
}

void async_acceptor::start_async_accept()
{
    auto sock = make_shared<tcp::socket>(ref(acceptor_.get_io_service()));
    auto peer = make_shared<tcp::endpoint>();

    acceptor_.async_accept(*sock, *peer, bind(&async_acceptor::on_accept, this, shared_from_this(), sock, peer, _1));
}

void async_acceptor::on_accept(acceptor_ptr that, shared_ptr<tcp::socket> socket, shared_ptr<tcp::endpoint> peer, error_code const& err)
{
    if (err)
    {
        if (err.category() == boost::system::system_category() &&
            err.value   () == boost::system::errc::operation_canceled)
        {
            that;
            cout << "accept operation was canceled" << endl;
            cout << "I've received this notification although I have alive acceptor" << endl;
        }
        else
            cout << "unexpected error: " << err.message() << endl;

        return;
    }

    start_async_accept();
    on_accept_(*socket, *peer, err);
}
