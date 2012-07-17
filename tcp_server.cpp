#include "common.h"
#include "connecting.h"
#include "tcp_service.h"

struct server
{
    server(io_service& io, size_t port)
        : acceptor_(async_acceptor::create(io, port, bind(&server::on_accepted, this, _1, _2, _3)))
        , timer_   (io)
    {
    }

    void on_accepted(tcp::socket& moveable_sock, tcp::endpoint const&, error_code const& /*err*/)
    {
        cout << "accepted some connection" << endl;

        socket_ = in_place(
            ref(moveable_sock),
            bind(&server::on_receive, this, _1, _2),
            bind(&server::on_disconnected, this),
            trace_error);

        acceptor_.reset();

        timer_.expires_from_now(boost::posix_time::milliseconds(9300));
        timer_.async_wait      (bind(&server::remove_waiting_socket, this, _1));
    }

    void on_receive(const void* data, size_t size)
    {
        assert(size == sizeof(test_msg_data));
        const test_msg_data* msg = reinterpret_cast<const test_msg_data*>(data);

        cout << "server has received: " << msg->counter << endl;
    }

    void on_disconnected()
    {
        socket_.reset();
        cout << "Hey client, I've lost you!" << endl;
    }

    void remove_waiting_socket(error_code const& code)
    {
        if (code)
        {
            cout << "supressing timer error" << endl;
            //trace_error(code);
            return;
        }

        socket_.reset();
    }

private:
    shared_ptr<async_acceptor>      acceptor_;
    optional<tcp_fragment_socket>   socket_;
    deadline_timer                  timer_;
};

void run_tcp_server(string /*server*/, size_t port)
{
    io_service io;
    server s(io, port);

    io.run();
}
