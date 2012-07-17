#include "common.h"
#include "connecting.h"

namespace network
{

void connect(io_service& io, string server, short port, on_connected_f const& on_connected, on_error_f const& on_error)
{
    struct async_connector
            : noncopyable
    {
        static void request(io_service& io, string server, short port, on_connected_f const& on_connected, on_error_f const& on_error)
        {
            // don't worry - the object will be deleted automatically after establishing connection
            ptr_t connector(new async_connector(io, on_connected, on_error));

            connector->sock_.async_connect(
                *asio_helper::first_endpoint<tcp>(io, server, port),
                bind(&async_connector::on_connected, connector, _1));

        }

    private:
        typedef shared_ptr<async_connector> ptr_t;

        async_connector(io_service& io, on_connected_f const& on_connected, on_error_f const& on_error)
            : sock_         (io)
            , on_connected_ (on_connected)
            , on_error_     (on_error )
        {
            assert(on_connected_);
        }

        void on_connected(error_code const& code)
        {
            if (code)
            {
                on_error_(code);
                return;
            }

            on_connected_(sock_);
        }


    private:
        tcp::socket     sock_;
        on_connected_f  on_connected_;
        on_error_f      on_error_;
    };

    async_connector::request(io, server, port, on_connected, on_error);
}

///////////////////////////////////////////////////////////////////////
// underlying_acceptor

struct underlying_acceptor
        : noncopyable
        , enable_shared_from_this<underlying_acceptor>
{
    typedef shared_ptr<underlying_acceptor>  ptr_t;

public:
    static ptr_t create(io_service& io, size_t port, network::on_accept_f const& on_accept, network::on_error_f const& on_error)
    {
        ptr_t acceptor(new underlying_acceptor(ref(io), port, boost::cref(on_accept), boost::cref(on_error)));
        acceptor->start_async_accept();

        return acceptor;
    }

    void stop_listening()
    {
        alive_ = false;
        acceptor_.cancel();
    }

private:
    underlying_acceptor(io_service& io, size_t port, network::on_accept_f const& on_accept, network::on_error_f const& on_error)

        : alive_    (true)
        , acceptor_ (io, tcp::endpoint(tcp::v4(), port))

        , on_accept_(on_accept)
        , on_error_ (on_error )
    {
    }

    void start_async_accept()
    {
        auto sock = make_shared<tcp::socket>(ref(acceptor_.get_io_service()));
        auto peer = make_shared<tcp::endpoint>();

        acceptor_.async_accept(*sock, *peer, bind(&underlying_acceptor::on_accept, shared_from_this(), sock, peer, _1));
    }

    void on_accept(shared_ptr<tcp::socket> socket, shared_ptr<tcp::endpoint> peer, error_code const& code)
    {
        if (!alive_)
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
    bool                    alive_;
    tcp::acceptor           acceptor_;
    network::on_accept_f    on_accept_;
    network::on_error_f     on_error_;
};


////////////////////////////////////////////////////////////////////////
// async_acceptor
async_acceptor::async_acceptor(io_service& io, size_t port, on_accept_f const& on_accept, on_error_f const& on_error)
    : acceptor_(underlying_acceptor::create(io, port, on_accept, on_error))
{
}

async_acceptor::~async_acceptor()
{
    acceptor_->stop_listening();
}

} // namespace network

