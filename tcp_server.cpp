#include "common.h"
#include "connecting.h"
#include "tcp_service.h"

struct server
{
    server(io_service& io, size_t port)
        : acceptor_(in_place(ref(io), port, bind(&server::on_accepted, this, _1, _2), trace_error))
        , timer_   (io)
    {
    }

    void on_accepted(tcp::socket& moveable_sock, tcp::endpoint const&)
    {
        cout << "accepted some connection" << endl;

        socket_ = in_place(
            ref(moveable_sock),
            bind(&server::on_receive, this, _1, _2),
            bind(&server::on_disconnected, this),
            trace_error);

        acceptor_.reset();

        timer_.expires_from_now(boost::posix_time::milliseconds(7300));
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
        cout << "Ooops! Client, I've lost you!" << endl;
    }

    void remove_waiting_socket(error_code const& code)
    {
        if (code)
        {
            cout << "supressing timer error" << endl;
            //trace_error(code);
            return;
        }

        cout << "Arrivederci client" << endl;
        socket_.reset();
    }

private:
    optional<network::async_acceptor>       acceptor_;
    optional<network::tcp_fragment_wrapper> socket_ ;
    deadline_timer                          timer_  ;
};

void run_tcp_server(string /*server*/, size_t port)
{
    io_service io;
    server s(io, port);

    io.run();
}
