#include "common.h"
#include "network.h"
#include "async_timer.h"

struct server
{
    server(io_service& io, size_t port)
        : acceptor_(in_place(ref(io), network::endpoint("", port), bind(&server::on_accepted, this, _1, _2), trace_error))
        , udp_sock_(
                in_place(ref(io),
                none,
                network::endpoint(
                    ip::address_v4::broadcast(),
                    port + 1),
                bind(&server::on_receive, this, _1, _2)/*network::on_receive_f()*/,
                trace_error))
        , timer_   (io, bind(&server::remove_waiting_socket, this))
    {
    }

    void on_accepted(tcp::socket& moveable_sock, network::endpoint const&)
    {
        cout << "accepted some connection" << endl;

        tcp_sock_ = in_place(
            ref(moveable_sock),
            bind(&server::on_receive, this, _1, _2),
            bind(&server::on_disconnected, this),
            trace_error);

        acceptor_.reset();
        timer_.wait(posix_time::milliseconds(7300));
    }

    void on_receive(const void* data, size_t size)
    {
        if (size != sizeof(test_msg_data))
            cout << "Expected size is: " << sizeof(test_msg_data) << "; received: " << size << endl;

        assert(size == sizeof(test_msg_data));
        const test_msg_data* msg = reinterpret_cast<const test_msg_data*>(data);

        cout << "server has received: " << msg->counter << endl;
        udp_sock_->send(msg, size);

        // check for deleteting from callback
        if (msg->counter > 155)
        {
//            udp_sock_.reset();
//            tcp_sock_.reset();
//            timer_.cancel_one();
        }
    }

    void on_disconnected()
    {
        tcp_sock_.reset();
        udp_sock_.reset();
        cout << "Ooops! Client, I've lost you!" << endl;
    }

    void remove_waiting_socket()
    {
        cout << "Arrivederci client" << endl;
        tcp_sock_.reset();
        udp_sock_.reset();
    }

private:
    optional<network::async_acceptor>       acceptor_;
    optional<network::tcp_fragment_wrapper> tcp_sock_;
    optional<network::udp_socket          > udp_sock_;
    async_timer                             timer_  ;
};

void run_tcp_server(string /*server*/, size_t port)
{
    io_service io;
    server s(io, port);

    io.run();
}
