#include "common.h"
#include "asio_helper.h"
#include "tcp_service.h"
#include "udp_service.h"

struct client
{
    client(io_service& io, string server, short port)
        : udp_sock_ (in_place(ref(io), network::endpoint("", port + 1), none, bind(&client::on_receive, this, _1, _2), trace_error))
        , timer_    (io)
    {
        network::connect(io, network::endpoint(server, port), bind(&client::on_connected, this, _1), trace_error);
    }

private:
    void on_connected(tcp::socket& sock)
    {
        cout << "Bingo, connected!" << endl;

        tcp_sock_ = in_place(
            ref(sock),
            bind(&client::on_receive, this, _1, _2),
            bind(&client::on_disconnected, this),
            trace_error);

        cock_the_clock(1);
    }

    void cock_the_clock(size_t seconds)
    {
        timer_.expires_from_now(boost::posix_time::seconds(seconds));
        timer_.async_wait(bind(&client::on_tick, this, _1));
    }

    void on_tick(const error_code& code)
    {
        if (code)
        {
            cout << "supressing timer error" << endl;
            //trace_error(code);
            return;
        }

        cock_the_clock(1);

        static size_t counter = 0;
        test_msg_data msg(++counter);

        tcp_sock_->send(&msg, sizeof(msg));
        cout << "client has sent: " << msg.counter << endl;
    }

    void on_receive(const void* data, size_t size)
    {
        assert(size == sizeof(test_msg_data));

        test_msg_data const& msg = *reinterpret_cast<test_msg_data const*>(data);
        cout << "Received back from server: " << msg.counter << endl;
    }

    void on_disconnected()
    {
        cout << "Olala, remote host has been disconnected" << endl;

        timer_.cancel();
        tcp_sock_ .reset();
        udp_sock_ .reset();
    }

private:
    optional<network::tcp_fragment_wrapper> tcp_sock_;
    optional<network::udp_socket          > udp_sock_;
    deadline_timer                          timer_;
};

void run_tcp_client(string server, size_t port)
{
    io_service io;
    client cl(io, server, port);

    io.run();
}


