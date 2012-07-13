#include "common.h"
#include "connecting.h"
#include "tcp_service.h"

struct server
{
    server(io_service& io, size_t port)
        : acceptor_(new async_acceptor(io, port, bind(&server::on_accepted, this, _1, _2, _3)))
    {
    }

    void on_accepted(tcp::socket& sock, tcp::endpoint const&, error_code const& /*err*/)
    {
        assert(!socket_);

        socket_  .reset(new tcp_fragment_socket(move(sock), bind(&server::on_receive, this, _1, _2)));
        acceptor_.reset();
    }

    void on_receive(const char* data, size_t size)
    {
        assert(size == sizeof(test_msg_data));
        const test_msg_data* msg = reinterpret_cast<const test_msg_data*>(data);

        cout << "server has received: " << msg->counter << endl;
    }

private:
    scoped_ptr<async_acceptor>        acceptor_;
    scoped_ptr<tcp_fragment_socket>   socket_;
};

void run_tcp_server(size_t port)
{
    io_service io;
    server s(io, port);

    io.run();
}
